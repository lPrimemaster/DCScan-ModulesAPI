#include "../include/DCS_ModuleAcquisition.h"
#include "../include/internal.h"

#include <NIDAQmx.h>

static std::map<DCS::DAQ::Task, DCS::DAQ::InternalTask> tasks_map;

static FILE* f_test = nullptr;

static DCS::i32 InternalNICallback(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData)
{
    LOG_DEBUG("Fired DCS internal acq callback.");

    if(f_test == nullptr)
    {
        f_test = fopen("Dump Data.txt", "w");
    }

    DCS::f64 samples[1000];
    DCS::i32 aread;

    DCS::u32 count[1000];
    DCS::i32 aread_c;

    DAQmxReadAnalogF64(taskHandle, nSamples, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, samples, nSamples, &aread, NULL);

    DAQmxReadCounterU32(taskHandle, nSamples, DAQmx_Val_WaitInfinitely, count, nSamples, &aread_c, NULL);

    for(int i = 0; i < 1000; i++)
    {
        std::string str = std::to_string(count[i]) + '\n';
        fwrite(str.c_str(), 1, str.size(), f_test);
    }

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
            SetupTask(tp, setup.clock.buffer, setup.clock_rate, InternalNICallback);
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
    if(f_test)
        fclose(f_test);
    f_test = nullptr;
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