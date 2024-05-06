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
         * \param taskHandle NIDAQmx handle.
         * \param everyNsamplesEventType NIDAQmx event type.
         * \param nSamples Number of samples to call recurring upon.
         * \param callbackData Back-end custom data passing to the NIDAQmx thread.
         * \return DCS::i32 Returns 0 upon no error.
         * 
         * \ingroup events
         */
        DCS_REGISTER_EVENT
        DCS::i32 CountEvent(TaskHandle taskHandle, DCS::i32 everyNsamplesEventType, DCS::u32 nSamples, void *callbackData);

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
         * \brief Get all the system present devices.
         */
        DCS_INTERNAL_TEST void GetDevices(char* buffer, u32 size);

        /**
         * \internal
         * \brief Create a task via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void CreateTask(InternalTask* t, const char* name);

        /**
         * \internal
         * \brief Setup an AI task (timing, channels, etc.) via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void SetupTaskAI(InternalTask* t, const char* clk_source, DCS::f64 clk, DCS::u64 num_samp, NIDataCallback func);

        /**
         * \internal
         * \brief Setup a CI task (timing, channels, etc.) via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void SetupTaskCI(InternalTask* t, const char* clk_source, DCS::f64 clk, DCS::u64 num_samp, NIDataCallback func);

        /**
         * \internal
         * \brief Setup a PTG task (timing, channels, etc.) via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void SetupTaskPTG(InternalTask* t, DCS::u64 buffer_sz);
        
        /**
         * \internal
         * \brief Add a channel to a task via NIDAQmx API.
         */
        DCS_INTERNAL_TEST void AddTaskChannel(InternalTask* t, const char* channel_name, ChannelType type, ChannelRef ref, ChannelLimits lims, const char* virtual_channel_name = nullptr, DCS::f64 implicit_rate = 0.0);

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
        DCS_INTERNAL_TEST DCS::DAQ::EventData GetLastDCS_IVD();

        /**
         * \internal
         * \brief Get last internal voltage data for the MCA system and pop from memory.
         */
        DCS_INTERNAL_TEST DCS::DAQ::EventData GetLastMCA_IVD();

        /**
         * \internal
         * \brief Get last internal voltage data for the Clinometer system and pop from memory.
         */
        DCS_INTERNAL_TEST DCS::DAQ::EventData GetLastClinometer_IVD();

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

    namespace Temp
    {
        /**
         * \internal
         * \brief Starts the temperature listener loop
         */
        DCS_INTERNAL_TEST void StartEventLoop(const char* com_port);

        /**
         * \internal
         * \brief Stops the temperature listener loop
         */
        DCS_INTERNAL_TEST void StopEventLoop();
    }

#ifndef NO_ENCODER_AVAILABLE
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
        DCS_INTERNAL_TEST void EIB7SoftModeLoopStart(DCS::f64 sigperiods[4]);

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
#endif //ENCODER_AVAILABLE
}
