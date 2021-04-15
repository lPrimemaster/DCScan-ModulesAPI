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
            Voltage
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

        struct /*DCS_API*/ ChannelLimits
        {
            DCS::f64 min = -10.0;
            DCS::f64 max =  10.0;
        };

        struct /*DCS_API*/ TaskSettings
        {
            DCS::Utils::BasicString task_name = { "" };

            DCS::Utils::BasicString channel0_name = { "" };
            DCS::Utils::BasicString channel1_name = { "" };
            DCS::Utils::BasicString channel2_name = { "" };
            DCS::Utils::BasicString channel3_name = { "" };
            DCS::Utils::BasicString channel4_name = { "" };

            ChannelRef channel0_ref = ChannelRef::None;
            ChannelRef channel1_ref = ChannelRef::None;
            ChannelRef channel2_ref = ChannelRef::None;
            ChannelRef channel3_ref = ChannelRef::None;
            ChannelRef channel4_ref = ChannelRef::None;

            ChannelType channel0_type = ChannelType::None;
            ChannelType channel1_type = ChannelType::None;
            ChannelType channel2_type = ChannelType::None;
            ChannelType channel3_type = ChannelType::None;
            ChannelType channel4_type = ChannelType::None;

            ChannelLimits channel0_lim;
            ChannelLimits channel1_lim;
            ChannelLimits channel2_lim;
            ChannelLimits channel3_lim;
            ChannelLimits channel4_lim;

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