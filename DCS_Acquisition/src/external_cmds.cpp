#include "../include/DCS_ModuleAcquisition.h"
#include "../include/internal.h"
#include "../../DCS_Network/include/internal.h"

#include <NIDAQmx.h>

static std::map<DCS::DAQ::Task, DCS::DAQ::InternalTask> tasks_map;

DCS::i32 DCS::DAQ::VoltageEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    DCS::f64 samples[1000];
    DCS::i32 aread;
    DAQmxReadAnalogF64(taskHandle, nSamples, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, samples, nSamples, &aread, NULL);

    //DCS::u32 count[1000];
    //DCS::i32 aread_c;
    //DAQmxReadCounterU32(taskHandle, nSamples, DAQmx_Val_WaitInfinitely, count, nSamples, &aread_c, NULL);

    // NOTE : Maybe use named event to reduce cpu time finding event name
    DCS_EMIT_EVT((DCS::u8*)samples, 1000 * sizeof(DCS::f64));
    return 0;
}

DCS::i32 DCS::DAQ::CounterEvent(DCS::u64 totalCount, DCS::u64 diffCount)
{
    DCS::u8 buffer[16];

    size_t u64s = sizeof(DCS::u64);

    memcpy(buffer       , &totalCount, u64s);
    memcpy(buffer + u64s, &diffCount , u64s);

    DCS_EMIT_EVT(buffer, (i32)u64s * 2);
    return 0;
}

static std::map<DCS::DAQ::Task, DCS::DAQ::InternalTask>::iterator FindByName(DCS::Utils::String name)
{
    return std::find_if(tasks_map.begin(), tasks_map.end(), [name](std::pair<DCS::DAQ::Task, DCS::DAQ::InternalTask> p) {
            if(strcmp(p.second.name.c_str(), name.c_str()) == 0)
                return true;
            return false;
        });
}

DCS::DAQ::Task DCS::DAQ::NewTask(DCS::DAQ::TaskSettings setup)
{
    LOG_DEBUG("Creating task: %s", setup.task_name.buffer);

    static Task l_task = 0;

    InternalTask t;

    InternalTask* tp = &t;

    CreateTask(tp, setup.task_name.buffer);

    // Setup all the possible channels
    for(int i = 0; i < 5; i++)
    {
        // Skip empty channel
        if(std::string(setup.channel_name[i].buffer) == "")
            continue;

        LOG_DEBUG("Adding channel %d", i);

        AddTaskChannel(tp,
            setup.channel_name[i].buffer, 
            setup.channel_type,
            setup.channel_ref[i],
            setup.channel_lim[i]);
    }

    switch(setup.channel_type)
    {
        case ChannelType::Voltage:
            SetupTask(tp, setup.clock.buffer, setup.clock_rate, VoltageEvent);
            break;
        case ChannelType::Counter:
            break;
        default:
            break;
    }

    tasks_map.emplace(l_task, t);

    LOG_DEBUG("Done");

    return l_task++;
}

void DCS::DAQ::StartTask(Task task)
{
    StartTask(&tasks_map.at(task));
}

void DCS::DAQ::StartNamedTask(DCS::Utils::BasicString task_name)
{
    auto it = FindByName(task_name.buffer);
    if(it == tasks_map.end())
    {
        LOG_ERROR("Could not find task named: %s", task_name.buffer);
    }
    else
    {
        StartTask(&it->second);
    }
}

void DCS::DAQ::StopTask(Task task)
{
    StopTask(&tasks_map.at(task));
}

void DCS::DAQ::StopNamedTask(DCS::Utils::BasicString task_name)
{
    auto it = FindByName(task_name.buffer);
    if(it == tasks_map.end())
    {
        LOG_ERROR("Could not find task named: %s", task_name.buffer);
    }
    else
    {
        StopTask(&it->second);
    }
}

void DCS::DAQ::DestroyTask(Task task)
{
    ClearTask(&tasks_map.at(task));
}

void DCS::DAQ::DestroyNamedTask(DCS::Utils::BasicString task_name)
{
    auto it = FindByName(task_name.buffer);
    if(it == tasks_map.end())
    {
        LOG_ERROR("Could not find task named: %s", task_name.buffer);
    }
    else
    {
        ClearTask(&it->second);
    }
}