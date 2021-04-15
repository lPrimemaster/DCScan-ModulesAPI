#pragma once
#include "DCS_ModuleAcquisition.h"

/**
 * @file
 * \internal
 * \brief Exposes internal functionalities of the DAQ namespace.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2021/04/13$
 */

namespace DCS
{
    namespace DAQ
    {
        typedef void* TaskHandle;

        typedef DCS::i32 (*NIDataCallback)(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData); 
        typedef DCS::i32 (*NIErrorCallback)(TaskHandle taskHandle, DCS::i32 status, void *callbackData);
        
        struct DCS_INTERNAL_TEST InternalChannel
        {
            ChannelType type;
            ChannelRef ref;
        };

        struct DCS_INTERNAL_TEST InternalTask
        {
            TaskHandle ni_opaque_handler;

            DCS::i32 clock_edge;
            DCS::f64 clock_rate;

            DCS::i32 sample_mode;
            DCS::u64 num_samples;

            NIDataCallback acq_callback;
            NIErrorCallback err_callback = nullptr;

            // TODO : Allocate data via some sort of pool for the caller thread to use safely
            DCS::u8* taskData = nullptr;

            DCS::Utils::String name;

            std::map<const char*, InternalChannel> vchannels;
        };

        DCS_INTERNAL_TEST void CreateTask(InternalTask* t, const char* name);

        DCS_INTERNAL_TEST void SetupTask(InternalTask* t, const char* clk_source, DCS::f64 clk, NIDataCallback func);

        DCS_INTERNAL_TEST void AddTaskChannel(InternalTask* t, const char* channel_name, ChannelType type, ChannelRef ref, ChannelLimits lims, const char* virtual_channel_name = nullptr);

        DCS_INTERNAL_TEST void StartTask(InternalTask* t);

        DCS_INTERNAL_TEST void StopTask(InternalTask* t);

        DCS_INTERNAL_TEST void ClearTask(InternalTask* t);

    }
}