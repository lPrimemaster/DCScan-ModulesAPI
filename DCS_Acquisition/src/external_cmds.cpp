#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"
#include "../../DCS_Utils/include/internal.h"

#include <queue>

#include <NIDAQmx.h>

struct VCData
{
    std::string channel_name;
    DCS::DAQ::ChannelType type;
    DCS::DAQ::ChannelRef ref;
    DCS::DAQ::ChannelLimits lim;
};

static DCS::DAQ::InternalTask voltage_task;
static DCS::Timer::SystemTimer voltage_task_timer;
static DCS::f64 voltage_task_rate;
static std::map<std::string, VCData> voltage_vcs;
static bool voltage_task_running = false;
static bool voltage_task_inited = false;

static std::queue<DCS::DAQ::InternalVoltageData> dcs_task_data;
static std::mutex dcs_mtx;
static std::condition_variable dcs_cv;

static std::queue<DCS::DAQ::InternalVoltageData> mca_task_data;
static std::mutex mca_mtx;
static std::condition_variable mca_cv;


static FILE* f;

// NOTE : Using circular buffer instead of allocating memory every time (?)
//static DCS::Memory::CircularBuffer crb(INTERNAL_SAMP_SIZE, 32);

// TODO : Create a global SendErrorToClient (also, check if it is being called from server-side only)

// TODO : Make the pushTo* functions to copy the data only once for all the tasks

static void pushToDCSTask(DCS::DAQ::InternalVoltageData& data)
{
    std::unique_lock<std::mutex> lck(dcs_mtx);
    dcs_task_data.push(data);
    lck.unlock();
    dcs_cv.notify_one();
}

static void pushToMCATask(DCS::DAQ::InternalVoltageData& data)
{
    std::unique_lock<std::mutex> lck(mca_mtx);
    mca_task_data.push(data);
    lck.unlock();
    mca_cv.notify_one();
}

void DCS::DAQ::Init()
{
    LOG_DEBUG("DAQ Services Running.");
    voltage_task_rate = 1000.0;
}

void DCS::DAQ::Terminate()
{
    LOG_DEBUG("Terminating DAQ Services.");
    voltage_task_inited = false;
}

static void InitAITask()
{
    LOG_DEBUG("Creating voltage task.");
    CreateTask(&voltage_task, "T_AI");
    voltage_task_inited = true;
    f = fopen("count_test_bins.csv", "w");
}

static void TerminateAITask()
{
    LOG_DEBUG("Terminating voltage task.");
    ClearTask(&voltage_task);
    voltage_task_inited = false;
    fclose(f);
}

// NOTE : This works because only one channel is being used. If more channels are used, this needs to be refactored.
DCS::i32 DCS::DAQ::VoltageEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    InternalVoltageData data;
    DCS::f64 samples[INTERNAL_SAMP_SIZE];
    DCS::i32 aread;

    data.timestamp = voltage_task_timer.getTimestamp();

    // TODO : Maybe try a more predictive approach if this is not good enough
    // IMPORTANT : To ensure usability take params -> (Nbuff / Fsamp) * Vtheta = delta_theta_min
    //             For this case -> (1000 / 100'000) * ([estimated?]~100 mdeg/s) = 1 mdeg uncertainty per buffer
    // TODO : Create a way to check for the ESP301 handle without any overhead. (Store value perhaps)
    //data.measured_angle = atof(DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::ESP301, { "2TP?" }).buffer);
    // TODO : Channels not in the task also queue in the buffer?  
    // TODO : Cout peaks in a separate thread if it gets slow in the live callback (FIFO Style as always =])

    DAQmxReadAnalogF64(taskHandle, nSamples, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, samples, nSamples, &aread, NULL);

    //memcpy(data.ptr, samples, INTERNAL_SAMP_SIZE * sizeof(f64));
    data.cr = DCS::Math::countArrayPeak(samples, INTERNAL_SAMP_SIZE, 0.2, 10.0, 0.0); // Copy data.cr

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

    // NOTE : Maybe use named event to reduce cpu time finding event name
    DCS_EMIT_EVT((DCS::u8*)samples, INTERNAL_SAMP_SIZE * sizeof(f64));
    return 0;
}

void DCS::DAQ::NewAIVChannel(DCS::Utils::BasicString name, DCS::Utils::BasicString channel_name, ChannelRef ref, ChannelLimits lim)
{
    for(auto vc : voltage_vcs)
    {
        if(vc.first == std::string(name.buffer))
        {
            LOG_ERROR("Voltage channel naming already exists. Choose another name.");
            return;
        }

        if(vc.second.channel_name == std::string(channel_name.buffer))
        {
            LOG_ERROR("Voltage channel hardware name already in use. Choose another connector.");
            return;
        }
    }
    
    LOG_DEBUG("Adding voltage channel %s to the voltage task.", channel_name.buffer);

    VCData vcd;
    vcd.channel_name = channel_name.buffer;
    vcd.ref = ref;
    vcd.lim = lim;
    vcd.type = ChannelType::Voltage;

    voltage_vcs.emplace(std::string(name.buffer), vcd);
}

void DCS::DAQ::DeleteAIVChannel(DCS::Utils::BasicString name)
{
    voltage_vcs.erase(std::string(name.buffer));
}

void DCS::DAQ::StartAIAcquisition(DCS::f64 samplerate)
{
    voltage_task_rate = samplerate;

    if(!voltage_task_running)
    {
        InitAITask();
        LOG_DEBUG("Sample rate set to: %.2f S/s.", voltage_task_rate);
        for(auto vcs : voltage_vcs)
        {
            VCData vchannel = vcs.second;
            AddTaskChannel(&voltage_task, vchannel.channel_name.c_str(), ChannelType::Voltage, vchannel.ref, vchannel.lim);
        }

        // Always use the internal clock for continuous acquisition
        SetupTask(&voltage_task, "OnBoardClock", voltage_task_rate, INTERNAL_SAMP_SIZE, VoltageEvent);
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
        TerminateAITask();
    }
    else
        LOG_ERROR("Error stopping AI Acquisition: VoltageAI task is not running.");
}

DCS::DAQ::InternalVoltageData DCS::DAQ::GetLastDCS_IVD()
{
    std::unique_lock<std::mutex> lck(dcs_mtx);
    
    if(dcs_task_data.empty())
        dcs_cv.wait(lck);

    InternalVoltageData ivd = dcs_task_data.front();
    dcs_task_data.pop();
    return ivd;
}

DCS::DAQ::InternalVoltageData DCS::DAQ::GetLastMCA_IVD()
{
    std::unique_lock<std::mutex> lck(mca_mtx);
    
    if(mca_task_data.empty())
        mca_cv.wait(lck);

    InternalVoltageData ivd = mca_task_data.front();
    mca_task_data.pop();
    return ivd;
}