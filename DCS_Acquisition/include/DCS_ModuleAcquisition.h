#ifndef _DCS_ACQ_H
#define _DCS_ACQ_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"


/**
 * @file
 * \brief Exposes DAQ functionalities of the API to the end user.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2021/04/12$
 */

// TODO : Document all this
namespace DCS
{
    namespace DAQ
    {
        typedef DCS::u32 Task;

        enum class ChannelType
        {
            None,
            Voltage,
            Counter
        };

        enum class ChannelRef
        {
            None               =      0,
            Default            =     -1, // DAQmx_Val_Cfg_Default
            SingleEnded        =  10083, // DAQmx_Val_RSE
            NoRefSingleEnded   =  10078, // DAQmx_Val_NRSE
            Differential       =  10106, // DAQmx_Val_Diff
            PseudoDifferential =  12529  // DAQmx_Val_PseudoDiff
        };

        struct DCS_API ChannelLimits
        {
            DCS::f64 min = -10.0;
            DCS::f64 max =  10.0;
        };

        struct DCS_API TaskSettings
        {
            DCS::Utils::BasicString task_name = { "" };
            DCS::Utils::BasicString channel_name[5] = { "" };
            

            // NOTE : Even though type is per channel. A task only supports channels of the same type 
            // (not true, but close enough for this application).
            ChannelType   channel_type   = ChannelType::None;
            ChannelRef    channel_ref[5] = { ChannelRef::None };
            ChannelLimits channel_lim[5];

            DCS::Utils::BasicString clock = { "OnBoardClock" };
            DCS::f64 clock_rate = 10000;

            // NOTE : The data is passed via a DCSModuleNetwork event
            DCS::u64 samples_per_event = 1000;
        };

        DCS_REGISTER_CALL(DCS::DAQ::Task, DCS::DAQ::TaskSettings)
        DCS_API Task NewTask(DCS::DAQ::TaskSettings setup);

        DCS_REGISTER_CALL(void, DCS::DAQ::Task)
        DCS_API void StartTask(Task task);

        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void StartNamedTask(DCS::Utils::BasicString task_name);

        DCS_REGISTER_CALL(void, DCS::DAQ::Task)
        DCS_API void StopTask(Task task);

        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void StopNamedTask(DCS::Utils::BasicString task_name);

        DCS_REGISTER_CALL(void, DCS::DAQ::Task)
        DCS_API void DestroyTask(Task task);

        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void DestroyNamedTask(DCS::Utils::BasicString task_name);
    }
}

#endif _DCS_ACQ_H