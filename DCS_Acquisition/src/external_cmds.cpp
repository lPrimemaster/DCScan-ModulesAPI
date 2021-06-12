#include "../include/DCS_ModuleAcquisition.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"

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
static std::queue<DCS::DAQ::InternalVoltageData> voltage_task_data;
static DCS::f64 voltage_task_rate;
static std::map<const char*, VCData> voltage_vcs;
static std::mutex voltage_mtx;
static std::condition_variable voltage_cv;
static bool voltage_task_running = false;
static bool voltage_task_inited = false;

// NOTE : Using circular buffer instead of allocating memory every time (?)
//static DCS::Memory::CircularBuffer crb(INTERNAL_SAMP_SIZE, 32);

// TODO : Create a global SendErrorToClient (also, check if it is being called from server-side only)

void DCS::DAQ::Init()
{
    LOG_DEBUG("DAQ Services Running.");
    voltage_task_rate = 1000.0;
}

void DCS::DAQ::Terminate()
{
    LOG_DEBUG("Terminating DAQ Services.");
    DCS::Timer::Delete(voltage_task_timer);
    voltage_task_inited = false;
}

static void InitAITask()
{
    LOG_DEBUG("Creating voltage task.");
    CreateTask(&voltage_task, "T_AI");
    voltage_task_inited = true;
}

static void TerminateAITask()
{
    LOG_DEBUG("Terminating voltage task.");
    ClearTask(&voltage_task);
    DCS::Timer::Delete(voltage_task_timer);
    voltage_task_inited = false;
}

// NOTE : This works because only one channel is being used. If more channels are used, this needs to be refactored
DCS::i32 DCS::DAQ::VoltageEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    InternalVoltageData data;
    DCS::f64 samples[INTERNAL_SAMP_SIZE];
    DCS::i32 aread;

    data.timestamp = DCS::Timer::GetTimestamp(voltage_task_timer);

    // TODO : Maybe try a more predictive approach if this is not good enough
    // IMPORTANT : To ensure usability take params -> (Nbuff / Fsamp) * Vtheta = delta_theta_min
    //             For this case -> (1000 / 100'000) * ([estimated?]~100 mdeg/s) = 1 mdeg uncertainty per buffer
    data.measured_angle = atof(DCS::Control::IssueGenericCommandResponse(DCS::Control::UnitTarget::ESP301, { "2TP?" }).buffer);

    DAQmxReadAnalogF64(taskHandle, nSamples, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, samples, nSamples, &aread, NULL);
    memcpy(data.ptr, samples, INTERNAL_SAMP_SIZE * sizeof(f64));

    std::unique_lock<std::mutex> lck(voltage_mtx);
    voltage_task_data.push(data);
    lck.unlock();
    voltage_cv.notify_one();

    // NOTE : Maybe use named event to reduce cpu time finding event name
    DCS_EMIT_EVT((DCS::u8*)samples, INTERNAL_SAMP_SIZE * sizeof(f64));
    return 0;
}

void DCS::DAQ::NewAIVChannel(DCS::Utils::BasicString name, DCS::Utils::BasicString channel_name, ChannelRef ref, ChannelLimits lim)
{
    for(auto vc : voltage_vcs)
    {
        if(std::string(vc.first) == std::string(name.buffer))
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

    voltage_vcs.emplace(name.buffer, vcd);
}

void DCS::DAQ::DeleteAIVChannel(DCS::Utils::BasicString name)
{
    voltage_vcs.erase(name.buffer);
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

        DCS::Timer::Delete(voltage_task_timer);
        StartTask(&voltage_task);
        voltage_task_timer = DCS::Timer::New();
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
        DCS::Timer::Delete(voltage_task_timer);
        voltage_task_running = false;
        TerminateAITask();
    }
    else
        LOG_ERROR("Error stopping AI Acquisition: VoltageAI task is not running.");
}

DCS::DAQ::InternalVoltageData DCS::DAQ::GetLastIVD()
{
    std::unique_lock<std::mutex> lck(voltage_mtx);
    
    if(voltage_task_data.empty())
        voltage_cv.wait(lck);

    InternalVoltageData ivd = voltage_task_data.front();
    voltage_task_data.pop();
    return ivd;
}