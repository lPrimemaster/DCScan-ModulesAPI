#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"
#include "../../DCS_Utils/include/internal.h"
#include "DCS_EngineControl/include/DCS_ModuleEngineControl.h"
#include "DCS_Utils/include/DCS_ModuleUtils.h"

#include <algorithm>
#include <chrono>
#include <queue>
#include <atomic>
#include <cstring>

#include <NIDAQmx.h>
#include <ratio>
#include <thread>

#undef max

struct VirtualChannel
{
    std::string channel_name;
    DCS::DAQ::ChannelType type;

    // Voltage only
    DCS::DAQ::ChannelRef ref;
    DCS::DAQ::ChannelLimits lim;

    // Counter only
    DCS::DAQ::ChannelCountRef count_ref;

    // PTGen only
    DCS::f64 ptg_rate;
};

static std::map<std::string, VirtualChannel> virtual_channels;
static std::map<int, int> virtual_channels_acq_order;

static DCS::DAQ::InternalTask voltage_task;
static DCS::Timer::SystemTimer voltage_task_timer;
static DCS::f64 voltage_task_rate;
static bool voltage_task_running = false;
static bool voltage_task_inited = false;

static DCS::DAQ::InternalTask count_task;
static DCS::Timer::SystemTimer count_task_timer;
static DCS::f64 count_task_rate;
static bool count_task_running = false;
static bool count_task_inited = false;
static bool count_using_ptg = false;
static DCS::u64 count_task_last_count = 0;

static DCS::DAQ::InternalTask ptg_task;

static std::queue<DCS::DAQ::EventData> dcs_task_data;
static std::mutex dcs_mtx;
static std::condition_variable dcs_cv;

static std::queue<DCS::DAQ::EventData> mca_task_data;
static std::mutex mca_mtx;
static std::condition_variable mca_cv;

static std::queue<DCS::DAQ::EventData> cli_task_data;
static std::mutex cli_mtx;
static std::condition_variable cli_cv;

static DCS::u64 task_time_real;

static std::atomic<DCS::u16> MCA_NumChannels = 2048;

static std::atomic<bool> measurement_control_running = false;
static std::atomic<bool> measurement_control_paused = false;
static std::atomic<DCS::i64> measurement_control_current_bin = -1;
static std::atomic<DCS::f64> measurement_control_current_bin_time = 0.0;
static std::atomic<DCS::i64> measurement_total_bins = 0;

// NOTE : Using circular buffer instead of allocating memory every time (?)
//static DCS::Memory::CircularBuffer crb(INTERNAL_SAMP_SIZE, 32);

// TODO : Create a global SendErrorToClient (also, check if it is being called from server-side only)

static DCS::i32 CountVirtualChannelsOfType(DCS::DAQ::ChannelType type)
{
    DCS::i32 total = 0;
    for(auto& channel : virtual_channels)
    {
        if(channel.second.type == type) total++;
    }
    return total;
}

static void pushToDCSTask(DCS::DAQ::EventData& data)
{
    std::unique_lock<std::mutex> lck(dcs_mtx);
    dcs_task_data.push(data);
    lck.unlock();
    dcs_cv.notify_one();
}

static void pushToMCATask(DCS::DAQ::EventData& data)
{
    std::unique_lock<std::mutex> lck(mca_mtx);
    mca_task_data.push(data);
    lck.unlock();
    mca_cv.notify_one();
}

static void pushToClinometerEvt(DCS::DAQ::EventData& data)
{
    std::unique_lock<std::mutex> lck(cli_mtx);
    cli_task_data.push(data);
    lck.unlock();
    cli_cv.notify_one();
}

static void SetVirtualChannelsIndices()
{
    virtual_channels_acq_order.clear();
    std::vector<int> channels;
    for(const auto& vc : virtual_channels)
    {
        if(vc.second.type == DCS::DAQ::ChannelType::Voltage)
        {
            size_t nidx = vc.second.channel_name.find_last_not_of("0123456789");
            channels.push_back(std::atoi(vc.second.channel_name.substr(nidx + 1).c_str()));
        }
    }
    std::sort(channels.begin(), channels.end());

    for(int i = 0; i < static_cast<int>(channels.size()); i++)
    {
        virtual_channels_acq_order.emplace(channels[i], i);
    }
}

static DCS::f64* GetSamplesOffsetForChannel(DCS::f64* samples, int chan)
{
    // BUG: (César) : All this just works with an even number of channels
    //                Because `INTERNAL_SAMP_SIZE` is 1000 (this should be a dynamic value instead)
    if(virtual_channels_acq_order.find(chan) == virtual_channels_acq_order.end())
    {
        LOG_ERROR("Channel %d was requested, but is not on the current task.", chan);
        return samples; // Avoid return nullptr
    }
    return &samples[(INTERNAL_SAMP_SIZE/virtual_channels_acq_order.size())*virtual_channels_acq_order[chan]];
}

void DCS::DAQ::Init()
{
    LOG_DEBUG("DAQ Services Running.");
    voltage_task_rate = 1000.0;
    count_task_rate = voltage_task_rate;
}

void DCS::DAQ::Terminate()
{
    LOG_DEBUG("Terminating DAQ Services.");
    voltage_task_inited = false;
}

// NOTE : This works because only one channel is being used. If more channels are used, this needs to be refactored.
DCS::i32 DCS::DAQ::VoltageEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    EventData data;
    DCS::f64 samples[INTERNAL_SAMP_SIZE];
    DCS::i32 samples_per_channel;

    // Software TS
    data.timestamp_wall = voltage_task_timer.getTimestamp();

    // TODO : Maybe try a more predictive approach if this is not good enough
    // IMPORTANT : To ensure usability take params -> (Nbuff / Fsamp) * Vtheta = delta_theta_min
    //             For this case -> (1000 / 100'000) * ([estimated?]~100 mdeg/s) = 1 mdeg uncertainty per buffer
    // TODO : Create a way to check for the ESP301 handle without any overhead. (Store value perhaps)
    //data.measured_angle = atof(DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::ESP301, { "2TP?" }).buffer);
    // TODO : Channels not in the task also queue in the buffer?  
    // TODO : Cout peaks in a separate thread if it gets slow in the live callback (FIFO Style as always =])
    
    DAQmxReadAnalogF64(taskHandle, -1, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, samples, INTERNAL_SAMP_SIZE, &samples_per_channel, NULL);

    // data.counts = DCS::Math::countArrayPeak(samples, samples_per_channel, 0.2, 10.0, 0.0); // Copy data.cr

    // TODO: (César) : Create a server-side function to set which channel is which
    data.tilt_c1[0] = DCS::Math::averageArray(GetSamplesOffsetForChannel(samples, 1), samples_per_channel);
    data.tilt_c1[1] = DCS::Math::averageArray(GetSamplesOffsetForChannel(samples, 2), samples_per_channel);

    // push to dcs
    if(DCS::Registry::CheckEvent(SV_EVT_DCS_DAQ_DCSCountEvent))
    {
        pushToDCSTask(data);
    }

    // push to mca
    if(DCS::Registry::CheckEvent(SV_EVT_DCS_DAQ_MCACountEvent))
    {
        pushToMCATask(data);
    }

    // push to clinometer display
    if(DCS::Registry::CheckEvent(SV_EVT_DCS_DAQ_ClinometerEvent))
    {
        pushToClinometerEvt(data);
    }

    // NOTE : Maybe use named event to reduce cpu time finding event name
    DCS_EMIT_EVT((DCS::u8*)samples, INTERNAL_SAMP_SIZE * sizeof(f64));
    return 0;
}

DCS::i32 DCS::DAQ::CountEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    EventData data;
    DCS::u32 counts[INTERNAL_SAMP_SIZE];
    DCS::i32 samples_per_channel;

    DAQmxReadCounterU32(taskHandle, -1, DAQmx_Val_WaitInfinitely, counts, INTERNAL_SAMP_SIZE, &samples_per_channel, NULL);

#ifdef FAKE_DATA
    data.counts.num_detected = 0;
    data.counts_delta        = 0;
    data.timestamp_wall      = count_task_timer.getTimestamp();
    task_time_real          += (u64)((INTERNAL_SAMP_SIZE / count_task_rate) * 1E9f);
    data.timestamp_real      = task_time_real;

    data.angle_c1       = 0.0;
    data.angle_c2       = 0.0;
    data.angle_detector = 0.0;
    data.angle_table    = 0.0;

    data.temp_c1                     = 0.0;
    data.temp_c2                     = 0.0;

    data.angle_eqv_bragg             = 0.0; // TODO
    data.lattice_spacing_uncorrected = 0.0; // TODO
    data.lattice_spacing_corrected   = 0.0; // TODO
    data.bin_number_uncorrected      = MControl::GetCurrentBin();
    data.bin_number_corrected        = 0;   // TODO
    data.bin_time                    = MControl::GetCurrentBinTime();

    LOG_WARNING("Sending fake data!");

    // push to dcs
    if(DCS::Registry::CheckEvent(SV_EVT_DCS_DAQ_DCSCountEvent))
    {
        pushToDCSTask(data);
    }
    
    DCS_EMIT_EVT((DCS::u8*)counts, INTERNAL_SAMP_SIZE * sizeof(u32));
    return 0;
#endif

    data.counts.num_detected = static_cast<DCS::u64>(counts[INTERNAL_SAMP_SIZE - 1]) - count_task_last_count;
    data.counts_delta        = counts[INTERNAL_SAMP_SIZE - 1] - counts[0];
    data.timestamp_wall      = count_task_timer.getTimestamp();
    task_time_real          += (u64)((INTERNAL_SAMP_SIZE / count_task_rate) * 1E9f);
    data.timestamp_real      = task_time_real;

    // Get the crystals position this frame
#ifdef NO_ENCODER_AVAILABLE
    data.angle_c1       = std::atof(Control::IssueGenericCommandResponse(Control::UnitTarget::ESP301, {"1PA?"}).buffer);
    data.angle_c2       = std::atof(Control::IssueGenericCommandResponse(Control::UnitTarget::ESP301, {"2PA?"}).buffer);
    data.angle_detector = std::atof(Control::IssueGenericCommandResponse(Control::UnitTarget::ESP301, {"3PA?"}).buffer);
    data.angle_table    = 0.0;
#else
    auto encoder_data   = ENC::InspectLastEncoderValues();
    data.angle_c1       = encoder_data.axis[1].calpos;
    data.angle_c2       = encoder_data.axis[3].calpos;
    data.angle_table    = Utils::String(Control::IssueGenericCommandResponse(
        Control::UnitTarget::XPSRLD4,
        { "GroupPositionCurrentGet(Table.Pos, double*)" }
    ).buffer).split(',')[1].tof64() + Database::ReadValuef64({"Geometric_AngleOffsetT"});
    data.angle_detector = Utils::String(Control::IssueGenericCommandResponse(
        Control::UnitTarget::XPSRLD4,
        { "GroupPositionCurrentGet(Detector.Pos, double*)" }
    ).buffer).split(',')[1].tof64() + Database::ReadValuef64({"Geometric_AngleOffsetD"});
#endif

    // Get the temperature this frame
    Temp::TemperatureData temp = Temp::InspectLastTemperatureValues();
    data.temp_c1                     = temp.crystals[0];
    data.temp_c2                     = temp.crystals[1];

    data.angle_eqv_bragg             = 0.0; // TODO
    data.lattice_spacing_uncorrected = 0.0; // TODO
    data.lattice_spacing_corrected   = 0.0; // TODO
    data.bin_number_uncorrected      = MControl::GetCurrentBin();
    data.bin_number_corrected        = 0;   // TODO
    data.bin_time                    = MControl::GetCurrentBinTime();

    count_task_last_count = counts[INTERNAL_SAMP_SIZE - 1];

    // push to dcs
    if(DCS::Registry::CheckEvent(SV_EVT_DCS_DAQ_DCSCountEvent))
    {
        pushToDCSTask(data);
    }

    // NOTE : Maybe use named event to reduce cpu time finding event name
    DCS_EMIT_EVT((DCS::u8*)counts, INTERNAL_SAMP_SIZE * sizeof(u32));
    return 0;
}

void DCS::DAQ::NewAIVChannel(DCS::Utils::BasicString name, DCS::Utils::BasicString channel_name, ChannelRef ref, ChannelLimits lim)
{
    for(auto vc : virtual_channels)
    {
        if(vc.first == std::string(name.buffer))
        {
            LOG_ERROR("Channel naming already exists. Choose another name.");
            return;
        }

        if(vc.second.channel_name == std::string(channel_name.buffer))
        {
            LOG_ERROR("Voltage channel hardware name already in use. Choose another connector.");
            return;
        }
    }
    
    LOG_DEBUG("Adding voltage channel %s to the voltage task.", channel_name.buffer);

    VirtualChannel vcd;
    vcd.channel_name = channel_name.buffer;
    vcd.ref = ref;
    vcd.lim = lim;
    vcd.type = ChannelType::Voltage;

    virtual_channels.emplace(std::string(name.buffer), vcd);
}

void DCS::DAQ::DeleteChannel(DCS::Utils::BasicString name)
{
    virtual_channels.erase(std::string(name.buffer));
}

void DCS::DAQ::StartAIAcquisition(DCS::Utils::BasicString clock_trigger_channel, DCS::f64 samplerate)
{
    if(count_task_running)
    {
        LOG_ERROR("Running CI and AI events in paralell is currently not supported.");
        return;
    }
    voltage_task_rate = samplerate;
    task_time_real = 0;
    if(!voltage_task_running)
    {
        StartEventLoop(MCA_NumChannels.load());

        LOG_DEBUG("Creating voltage task.");
        CreateTask(&voltage_task, "T_AI");
        LOG_DEBUG("Sample rate set to: %.2f S/s.", voltage_task_rate);
        voltage_task_inited = true;
        for(auto vcs : virtual_channels)
        {
            VirtualChannel& vchannel = vcs.second;
            
            if(vchannel.type != ChannelType::Voltage) continue;
            AddTaskChannel(&voltage_task, vchannel.channel_name.c_str(), ChannelType::Voltage, vchannel.ref, vchannel.lim);
        }

        // Always use the internal clock for continuous acquisition
        SetVirtualChannelsIndices();
        SetupTaskAI(&voltage_task, clock_trigger_channel.buffer, voltage_task_rate, INTERNAL_SAMP_SIZE / CountVirtualChannelsOfType(DCS::DAQ::ChannelType::Voltage), VoltageEvent);
        StartTask(&voltage_task);
        voltage_task_timer.start();
        voltage_task_running = true;
    }
    else
        LOG_ERROR("Error starting AI Acquisition: VoltageAI task is already running.");
}

void DCS::DAQ::StopAIAcquisition()
{
    if(voltage_task_running)
    {
        StopTask(&voltage_task);
        voltage_task_running = false;
        LOG_DEBUG("Terminating voltage task.");
        ClearTask(&voltage_task);
        voltage_task_inited = false;
        StopEventLoop();
    }
    else
        LOG_ERROR("Error stopping AI Acquisition: VoltageAI task is not running.");
}

void DCS::DAQ::NewCIChannel(DCS::Utils::BasicString name, DCS::Utils::BasicString channel_name, ChannelCountRef ref)
{
    for(auto vc : virtual_channels)
    {
        if(vc.first == std::string(name.buffer))
        {
            LOG_ERROR("Channel naming already exists. Choose another name.");
            return;
        }

        if(vc.second.channel_name == std::string(channel_name.buffer))
        {
            LOG_ERROR("Counter channel hardware name already in use. Choose another connector.");
            return;
        }
    }
    
    LOG_DEBUG("Adding counter channel %s to the counter task.", channel_name.buffer);

    VirtualChannel vcd;
    vcd.channel_name = channel_name.buffer;
    vcd.count_ref = ref;
    vcd.type = ChannelType::Counter;

    virtual_channels.emplace(std::string(name.buffer), vcd);
}

void DCS::DAQ::NewPTGChannel(DCS::Utils::BasicString name, DCS::Utils::BasicString channel_name, f64 rate)
{
    for(auto vc : virtual_channels)
    {
        if(vc.first == std::string(name.buffer))
        {
            LOG_ERROR("Channel naming already exists. Choose another name.");
            return;
        }

        if(vc.second.channel_name == std::string(channel_name.buffer))
        {
            LOG_ERROR("PTG channel hardware name already in use. Choose another connector.");
            return;
        }
    }
    
    LOG_DEBUG("Adding PTG channel %s to the PTG task.", channel_name.buffer);

    VirtualChannel vcd;
    vcd.channel_name = channel_name.buffer;
    vcd.ptg_rate = rate;
    vcd.type = ChannelType::PTGen;

    virtual_channels.emplace(std::string(name.buffer), vcd);
}

void DCS::DAQ::StartCIAcquisition(DCS::Utils::BasicString clock_trigger_channel, DCS::Utils::BasicString pause_trigger_channel, DCS::Utils::BasicString reset_trigger_channel, f64 rate)
{
    if(voltage_task_running)
    {
        LOG_ERROR("Running CI and AI events in paralell is currently not supported.");
        return;
    }
    if(!count_task_running)
    {
        task_time_real = 0;
        count_task_last_count = 0;
        StartEventLoop(0);

        LOG_DEBUG("Creating counter task.");
        CreateTask(&count_task, "T_CI");
        count_task_inited = true;
        DCS::i32 channel_count = 0;
        count_task_rate = rate;
        std::string last_channel;
        for(auto vcs : virtual_channels)
        {
            VirtualChannel& vchannel = vcs.second;
            
            if(vchannel.type == ChannelType::Counter)
            {
                if(++channel_count > 1)
                {
                    LOG_ERROR("CI Acquisition only supports one channel per task.");
                    LOG_ERROR("Using first channel found -> %s.", last_channel.c_str());
                    break;
                }
                AddTaskChannel(&count_task, vchannel.channel_name.c_str(), ChannelType::Counter, {}, {});
                last_channel = vchannel.channel_name;
            }
        }

        channel_count = 0;
        bool using_ptg = false;
        for(auto vcs : virtual_channels)
        {
            VirtualChannel& vchannel = vcs.second;
            
            if(vchannel.type == ChannelType::PTGen)
            {
                if(++channel_count > 1)
                {
                    LOG_ERROR("PTG only supports one channel per task.");
                    LOG_ERROR("Using first channel found -> %s.", last_channel.c_str());
                    break;
                }
                
                LOG_MESSAGE("Found a PTG Channel created. Setting up PTG for the timebase.");
                LOG_DEBUG("Creating ptg task.");
                CreateTask(&ptg_task, "T_PTG");
                last_channel = vchannel.channel_name;
                AddTaskChannel(&ptg_task, vchannel.channel_name.c_str(), ChannelType::PTGen, {}, {}, nullptr, count_task_rate);
                SetupTaskPTG(&ptg_task, 1000); // or INTERNAL_SAMP_SIZE?
                count_using_ptg = true;
            }
        }


        if(strcmp(pause_trigger_channel.buffer, "NONE") != 0)
        {
            // TODO
            LOG_WARNING("The pause trigger functionality is currently not implemented.");
        }
        
        if(strcmp(reset_trigger_channel.buffer, "NONE") != 0)
        {
            // TODO
            LOG_WARNING("The reset trigger functionality is currently not implemented.");
        }

        LOG_DEBUG("Setting up count task.");
        SetupTaskCI(&count_task, clock_trigger_channel.buffer, count_task_rate, INTERNAL_SAMP_SIZE, CountEvent);
        if(count_using_ptg) StartTask(&ptg_task);
        StartTask(&count_task);
        count_task_timer.start();
        count_task_running = true;
    }
    else
        LOG_ERROR("Error starting CI Acquisition: CI task is already running.");
}

void DCS::DAQ::StopCIAcquisition()
{
    if(count_task_running)
    {
        StopTask(&count_task);
        if(count_using_ptg) StopTask(&ptg_task);
        count_task_running = false;
        LOG_DEBUG("Terminating CI task.");
        ClearTask(&count_task);
        if(count_using_ptg) ClearTask(&ptg_task);
        count_task_inited = false;
        count_using_ptg = false;
        StopEventLoop();
    }
    else
        LOG_ERROR("Error stopping CI Acquisition: Counter task is not running.");
}

void DCS::DAQ::StartDIAcquisition(DCS::f64 samplerate)
{
    // TODO
}

void DCS::DAQ::StopDIAcquisition()
{
    // TODO
}

DCS::DAQ::EventData DCS::DAQ::GetLastDCS_IVD()
{
    std::unique_lock<std::mutex> lck(dcs_mtx);
    
    if(dcs_task_data.empty())
        dcs_cv.wait(lck);

    if(!dcs_task_data.empty())
    {
        EventData ivd = dcs_task_data.front();
        dcs_task_data.pop();
        return ivd;
    }
    else
    {
        // Notify unlock
        EventData ivd_nu;
        ivd_nu.counts.num_detected = std::numeric_limits<u64>::max();
        return ivd_nu;
    }
}

DCS::DAQ::EventData DCS::DAQ::GetLastMCA_IVD()
{
    std::unique_lock<std::mutex> lck(mca_mtx);
    
    if(mca_task_data.empty())
        mca_cv.wait(lck);

    if(!mca_task_data.empty())
    {
        EventData ivd = mca_task_data.front();
        mca_task_data.pop();
        return ivd;
    }
    else
    {
        // Notify unlock
        EventData ivd_nu;
        ivd_nu.counts.num_detected = std::numeric_limits<u64>::max();
        return ivd_nu;
    }
}

DCS::DAQ::EventData DCS::DAQ::GetLastClinometer_IVD()
{
    std::unique_lock<std::mutex> lck(cli_mtx);
    
    if(cli_task_data.empty())
        cli_cv.wait(lck);

    if(!cli_task_data.empty())
    {
        EventData ivd = cli_task_data.front();
        cli_task_data.pop();
        return ivd;
    }
    else
    {
        // Notify unlock
        EventData ivd_nu;
        ivd_nu.counts.num_detected = std::numeric_limits<u64>::max();
        return ivd_nu;
    }
}

DCS::u16 DCS::DAQ::GetMCANumChannels()
{
    return MCA_NumChannels.load();
}

void DCS::DAQ::SetMCANumChannels(DCS::u16 nChannels)
{
    u16 c = nChannels;
    if(nChannels > INTERNAL_ADC_MAX_CHAN)
    {
        c = MCA_NumChannels.load();
        LOG_ERROR("Attempting to set MCA channels to a value larger than INTERNAL_ADC_MAX_CHAN (%u).", INTERNAL_ADC_MAX_CHAN);
    }
    MCA_NumChannels.store(c);
    LOG_MESSAGE("MCA Channel number set to: %u", c);
}

DCS::f64 DCS::DAQ::GetADCMaxInternalClock()
{
    return INTERNAL_ADC_MAX_CLK; 
}

DCS::Utils::BasicString DCS::DAQ::GetConnectedDevicesAliases()
{
    DCS::Utils::BasicString string;
    DCS::DAQ::GetDevices(string.buffer, sizeof(string.buffer));
    return string;
}

void DCS::DAQ::NotifyUnblockEventLoop()
{
    mca_cv.notify_one();
    dcs_cv.notify_one();
    cli_cv.notify_one();
}

void DCS::MControl::StartMeasurementIncTimebased(const MeasurementRoutineData& data)
{
    if(!measurement_control_running.load())
    {
        measurement_control_running.store(true);
        std::thread([](MeasurementRoutineData data) {
            // Orquestrate everything into it's starting position!
            // The detector does not need PID movement
            DCS::Control::MoveAbsolute(Control::UnitTarget::XPSRLD4, { "Detector" }, data.detector_ref);

            // THe table does not need PID movement
            DCS::Control::MoveAbsolute(Control::UnitTarget::XPSRLD4, { "Table" }, data.table_ref);

            // C1 and C2 use PID for precise position control
            DCS::Control::MoveAbsolutePID(Control::UnitTarget::XPSRLD4, { "Crystal1" }, data.c1_ref);
            DCS::Control::MoveAbsolutePID(Control::UnitTarget::XPSRLD4, { "Crystal2" }, data.c2_start);

            // Wait a few moments to stabilize
            std::this_thread::sleep_for(std::chrono::seconds(2));

            // Ready to start the control subroutine
            // Run every few milliseconds (ideally a factor of ~10 lower than the bin time)
            measurement_total_bins = static_cast<i64>(std::abs(data.c2_stop - data.c2_start) / data.bin_width);
            const f64 sign       = (data.c2_stop >= data.c2_start) ? 1.0 : -1.0;
            const i64 bin_nanos  = static_cast<i64>(data.bin_time * 1E9);
            Timer::SystemTimer timer;
            while(measurement_control_current_bin.load() < measurement_total_bins)
            {
                timer.start();
                measurement_control_current_bin++;
                DCS_EMIT CurrentMeasurementProgressChangedEvent();
                i64 tleft = bin_nanos - timer.getNanoseconds();
                while(tleft > 1000) // 1 us increments
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1000));

                    // Check if we want to stop
                    if(!measurement_control_running.load()) break;

                    i64 curr_nanos = timer.getNanoseconds();

                    // Only update time left if we are not paused
                    if(!measurement_control_paused.load())
                    {
                        tleft = bin_nanos - curr_nanos;
                    }
                    
                    measurement_control_current_bin_time.store(curr_nanos / 1E9);
                }

                // Check if we want to stop
                if(!measurement_control_running.load()) break;

                // For now we can use a relative move approach and see how good it is
                DCS::Control::MoveRelative(Control::UnitTarget::XPSRLD4, { "Crystal2" }, data.bin_width * sign);
            }
            measurement_control_current_bin.store(-1);
            measurement_control_current_bin_time.store(0.0);
            measurement_control_running.store(false);
            DCS_EMIT MeasurementControlRoutineEnded();
        }, data).detach();
    }
    else
    {
        LOG_WARNING("A measurement service is already running.");
    }
}

void DCS::MControl::StartMeasurementControlRoutine(MeasurementRoutineData data)
{
    // The measurement and control service is running on a different thread
    // We should be able to start/stop/pause this service whenever necessary
    // This function handles the starting part

    switch(data.mode)
    {
    case MeasurementRoutineMode::INC_TIME_BASED:
        StartMeasurementIncTimebased(data);
        break;
    case MeasurementRoutineMode::INC_EVENT_BASED:
        LOG_WARNING("Incemental Event Based Measurement Control is not available.");
        break;
    case MeasurementRoutineMode::SWP_TIME_BASED:
        LOG_WARNING("Sweep Time Based Measurement Control is not available.");
        break;
    default:
        LOG_ERROR("Unknown MeasurementRoutineMode.");
        break;
    }
}

DCS_API void DCS::MControl::StopMeasurementControlRoutine()
{
    if(measurement_control_running.load())
    {
        measurement_control_running.store(false);
    }
    else
    {
        LOG_WARNING("No Measurement Control Routine is running.");
    }
}

DCS_API void DCS::MControl::PauseMeasurementControlRoutine()
{
    if(measurement_control_running.load())
    {
        measurement_control_paused.store(true);
    }
    else
    {
        LOG_WARNING("No Measurement Control Routine is running.");
    }
}

DCS_API void DCS::MControl::ResumeMeasurementControlRoutine()
{
    if(measurement_control_running.load() && measurement_control_paused.load())
    {
        measurement_control_paused.store(false);
    }
    else
    {
        LOG_WARNING("No Measurement Control Routine is paused.");
    }
}

DCS_API DCS::i64 DCS::MControl::GetCurrentBin()
{
    return measurement_control_current_bin.load();
}

DCS_API DCS::f64 DCS::MControl::GetCurrentBinTime()
{
    return measurement_control_current_bin_time.load();
}

DCS_API DCS::i64 DCS::MControl::GetCurrentMeasurementProgress()
{
    const i64 progress = (measurement_control_current_bin.load() * 100) / measurement_total_bins.load();
    return progress < 0 ? 0 : progress;
}
