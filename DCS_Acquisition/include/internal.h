#pragma once
#include "DCS_ModuleAcquisition.h"
#include "../../config/exports.h"

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

// TODO : Pass config values to a config section for ease of modification
#define INTERNAL_SAMP_SIZE 1000
#define INTERNAL_ADC_MAX_CHAN 65535
#define INTERNAL_ADC_MAX_CLK 250000.0

namespace DCS
{
    namespace DAQ
    {
        /**
         * \internal
         * \brief Forward declare the NIDAQmx TaskHandle opaque type. 
         */
        typedef void* TaskHandle;

        /**
         * \internal
         * \brief Internally checks for n voltage data points.
         * 
         * \param taskHandle NIDAQmx handle.
         * \param everyNsamplesEventType NIDAQmx event type.
         * \param nSamples Number of samples to call recurring upon.
         * \param callbackData Back-end custom data passing to the NIDAQmx thread.
         * \return DCS::i32 Returns 0 upon no error.
         * 
         * \ingroup events
         */
        DCS_REGISTER_EVENT
        DCS::i32 VoltageEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData);

        /**
         * \internal
         * \brief Internally checks for n voltage data points.
         * \todo Implement.
         * \param totalCount Total counts got from the NIDAQmx since counter task start.
         * \param diffCount Count difference from last emission of the event.
         * \return DCS::i32 Returns 0 upon no error.
         * \todo Implement.
         */
        //DCS_REGISTER_EVENT
        //DCS::i32 CounterEvent(DCS::u64 totalCount, DCS::u64 diffCount);

        /**
         * \internal
         * \brief NIDAQmx internal API onData callback.
         */
        typedef DCS::i32 (*NIDataCallback)(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData); 

        /**
         * \internal
         * \brief NIDAQmx internal API onError callback.
         */
        typedef DCS::i32 (*NIErrorCallback)(TaskHandle taskHandle, DCS::i32 status, void *callbackData);
        
        /**
         * \internal
         * \brief Stores channel information for later configuration.
         */
        struct DCS_INTERNAL_TEST InternalChannel
        {
            ChannelType type;
            ChannelRef ref;
        };

        /**
         * \internal
         * \brief Stores task information for later configuration.
         */
        struct DCS_INTERNAL_TEST InternalTask
        {
            TaskHandle ni_opaque_handler;

            DCS::i32 clock_edge;
            DCS::f64 clock_rate;

            DCS::i32 sample_mode;
            DCS::u64 num_samples;

            NIDataCallback acq_callback;
            NIErrorCallback err_callback = nullptr;

            DCS::u8* taskData = nullptr;

            DCS::Utils::String name;

            std::map<const char*, InternalChannel> vchannels;
        };

        /**
         * \internal
         * \brief Holds data from a single VoltageEvent callback.
         */
        struct DCS_INTERNAL_TEST InternalVoltageData
        {
            f64 ptr[INTERNAL_SAMP_SIZE];
            Math::CountResult cr;

            Timer::Timestamp timestamp;

            f64 measured_angle;
            f64 predicted_angle;
        };

        /**
         * \internal
         * \brief Create a task via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void CreateTask(InternalTask* t, const char* name);

        /**
         * \internal
         * \brief Setup a task (timing, channels, etc.) via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void SetupTask(InternalTask* t, const char* clk_source, DCS::f64 clk, DCS::u64 num_samp, NIDataCallback func);

        /**
         * \internal
         * \brief Add a channel to a task via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void AddTaskChannel(InternalTask* t, const char* channel_name, ChannelType type, ChannelRef ref, ChannelLimits lims, const char* virtual_channel_name = nullptr);

        /**
         * \internal
         * \brief Tell the NIDAQmx API to start the task.
         */
        DCS_INTERNAL_TEST void StartTask(InternalTask* t);

        /**
         * \internal
         * \brief Tell the NIDAQmx API to stop the task.
         */
        DCS_INTERNAL_TEST void StopTask(InternalTask* t);

        /**
         * \internal
         * \brief Tell the NIDAQmx API to clear the task.
         */
        DCS_INTERNAL_TEST void ClearTask(InternalTask* t);

        /**
         * \internal
         * \brief Get last internal voltage data for the DCS system and pop from memory.
         */
        DCS_INTERNAL_TEST DCS::DAQ::InternalVoltageData GetLastDCS_IVD();

        /**
         * \internal
         * \brief Get last internal voltage data for the MCA system and pop from memory.
         */
        DCS_INTERNAL_TEST DCS::DAQ::InternalVoltageData GetLastMCA_IVD();

        /**
         * \internal
         * \brief Starts the DCS and MCA's event listener on the server side 
         */
        DCS_INTERNAL_TEST void StartEventLoop(u16 mca_num_channels);

        /**
         * \internal
         * \brief Stops the DCS and MCA's event listener on the server side 
         */
        DCS_INTERNAL_TEST void StopEventLoop();

        /**
         * \internal
         * \brief Notify callbacks about eventloop stop.
         */
        DCS_INTERNAL_TEST void NotifyUnblockEventLoop();
    }

    namespace ENC
    {
        /**
         * \internal
         * \brief Loads an encoder with controller ip and axes numbers to read.
         */
        DCS_INTERNAL_TEST void InitEIB7Encoder(const char* hostname, i8 axes);
        
        /**
         * \internal
         * \brief Starts encoder trigger.
         */
        DCS_INTERNAL_TEST void StartEIB7SoftModeTrigger();

        /**
         * \internal
         * \brief Starts encoder data acquisition.
         */
        DCS_INTERNAL_TEST void EIB7SoftModeLoopStart();

        /**
         * \internal
         * \brief Stops encoder data acquisition.
         */
        DCS_INTERNAL_TEST void EIB7SoftModeLoopStop();

        /**
         * \internal
         * \brief Stops encoder trigger.
         */
        DCS_INTERNAL_TEST void StopEIB7SoftModeTrigger();

        /**
         * \internal
         * \brief Closes the encoder.
         */
        DCS_INTERNAL_TEST void DeleteEIB7Encoder();
    }
}