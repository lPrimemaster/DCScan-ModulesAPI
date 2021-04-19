#include "../include/DCS_ModuleAcquisition.h"
#include "../include/internal.h"
#include "../../DCS_Utils/include/internal.h"

#include <NIDAQmx.h>

static bool HandleNiError(DCS::i32 error)
{
    if(error < 0) // Error
    {
        char buffer[2048];
		DAQmxGetExtendedErrorInfo(buffer, 2048);

        LOG_ERROR("NI Device: %s", buffer);

        return false;
    }
    else if(error > 0) // Warning
    {
        char buffer[2048];
		DAQmxGetExtendedErrorInfo(buffer, 2048);

        LOG_WARNING("NI Device: %s", buffer);

        return true;
    }
    else // Gucci
    {
        return true;
    }
}

void DCS::DAQ::CreateTask(InternalTask* t, const char* name)
{
    if(t == nullptr)
    {
        LOG_ERROR("Cannot initialize an empty task.");
    }
    else
    {
        t->name = name;
        DCS::i32 err = DAQmxCreateTask(name, &t->ni_opaque_handler);
        if(!HandleNiError(err))
        {
            LOG_ERROR("Error creating task.");
            DAQmxClearTask(t->ni_opaque_handler);
        }
    }
}

void DCS::DAQ::SetupTask(InternalTask* t, const char* clk_source, DCS::f64 clk, NIDataCallback func)
{
    DCS::f64 maxrate;
	DCS::i32 err = DAQmxGetSampClkMaxRate(t->ni_opaque_handler, &maxrate);

    if(!HandleNiError(err))
    {
        LOG_ERROR("Error getting device max sample clock rate.");
        DAQmxClearTask(t->ni_opaque_handler);
        return;
    }

    if(clk > maxrate)
    {
        LOG_WARNING("The requested task clock sample rate could not be assigned.");
        LOG_WARNING("Reason: Requested clock rate is larger than the max hardware rate limit.");
        LOG_MESSAGE("Setting the task clock rate to max OnBoardClock rate: %lf", maxrate);
    }

    // TODO : Make these arguments
    t->clock_edge  = DAQmx_Val_Rising;
    t->clock_rate  = clk > maxrate ? maxrate : clk;
    t->sample_mode = DAQmx_Val_ContSamps;
    t->num_samples = 1000;
    t->acq_callback = func;

    err = DAQmxCfgSampClkTiming(t->ni_opaque_handler, clk_source, t->clock_rate, t->clock_edge, t->sample_mode, t->num_samples);

    if(!HandleNiError(err))
    {
        LOG_ERROR("Error setting device clock timing settings.");
        DAQmxClearTask(t->ni_opaque_handler);
        return;
    }

    err = DAQmxRegisterEveryNSamplesEvent(t->ni_opaque_handler, DAQmx_Val_Acquired_Into_Buffer, (DCS::u32)t->num_samples, 0, t->acq_callback, t->taskData);
	if (!HandleNiError(err))
	{
		LOG_ERROR("Error setting callback function for task.");
        DAQmxClearTask(t->ni_opaque_handler);
        return;
	}

    if(t->err_callback != nullptr)
    {
        err = DAQmxRegisterDoneEvent(t->ni_opaque_handler, 0, t->err_callback, nullptr);
        if (!HandleNiError(err))
        {
            LOG_ERROR("Error setting termination callback function for task.");
        }
    }
}

void DCS::DAQ::AddTaskChannel(InternalTask* t, const char* channel_name, ChannelType type, ChannelRef ref, ChannelLimits lims, const char* virtual_channel_name)
{
    switch (type)
    {
    case ChannelType::Voltage:
    {
        DCS::i32 err = DAQmxCreateAIVoltageChan(t->ni_opaque_handler, 
                                                channel_name, 
                                                virtual_channel_name, 
                                                DCS::Utils::toUnderlyingType(ref),
                                                lims.min, lims.max,
                                                DAQmx_Val_Volts,
                                                NULL);
        
        if(!HandleNiError(err))
        {
            LOG_ERROR("Error setting virtual voltage channel.");
            DAQmxClearTask(t->ni_opaque_handler);
            return;
        }
    }
    break;

    case ChannelType::Counter:
    {
        DCS::i32 err = DAQmxCreateCICountEdgesChan(t->ni_opaque_handler,
                                                channel_name,
                                                virtual_channel_name,
                                                DAQmx_Val_Falling,
                                                0,
                                                DAQmx_Val_CountUp);
        
        if(!HandleNiError(err))
        {
            LOG_ERROR("Error setting virtual counter channel.");
            DAQmxClearTask(t->ni_opaque_handler);
            return;
        }
    }
    break;

    default:
        LOG_ERROR("Channel type for AddTaskChannel not recognized.");
        break;
    }
}

void DCS::DAQ::StartTask(InternalTask* t)
{
    DCS::i32 err = DAQmxStartTask(t->ni_opaque_handler);
	if (!HandleNiError(err))
	{
		LOG_ERROR("Error starting task.");
	}
}

void DCS::DAQ::StopTask(InternalTask* t)
{
    DCS::i32 err = DAQmxStopTask(t->ni_opaque_handler);
	if (!HandleNiError(err))
	{
		LOG_ERROR("Error stopping task.");
	}
}

void DCS::DAQ::ClearTask(InternalTask* t)
{
    DCS::i32 err = DAQmxClearTask(t->ni_opaque_handler);
	if (!HandleNiError(err))
	{
		LOG_ERROR("Error clearing task.");
	}
}