#include "../include/DCS_ModuleAcquisition.h"
#include "../include/internal.h"

#include <NIDAQmx.h>

static std::map<DCS::DAQ::Task, DCS::DAQ::InternalTask> tasks_map;

static DCS::i32 InternalNICallback(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    LOG_DEBUG("Fired DCS internal acq callback.");

    // TODO : Call user defined callbacks here
    
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

    LOG_DEBUG("Waiting task creation");
    CreateTask(tp, setup.task_name.buffer);
    LOG_DEBUG("Task creation done");

    // Setup all the possible channels
    for(int i = 0; i < 5; i++)
    {
        // Skip empty channel
        if(setup.channel_name[i].buffer == "" || setup.channel_type[i] == ChannelType::None)
            continue;

        LOG_DEBUG("Adding channel %d", i);

        AddTaskChannel(tp,
            setup.channel_name[i].buffer, 
            setup.channel_type[i],
            setup.channel_ref[i],
            setup.channel_lim[i]);
    }

    SetupTask(tp, setup.clock.buffer, setup.clock_rate, InternalNICallback);

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