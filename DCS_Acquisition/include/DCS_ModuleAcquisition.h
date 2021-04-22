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

namespace DCS
{
    /**
     * \brief Exposes %DAQ functionalities of the API to the end user.
     */
    namespace DAQ
    {
        /**
         * \brief Task opaque identifier. 
         */
        typedef DCS::u32 Task;

        /**
         * \brief Classifies a channel type of the DAQ.
         */
        enum class ChannelType
        {
            None,    ///< No channel.
            Voltage, ///< A voltage channel.
            Counter  ///< A counter channel. Freq out for outputs.
        };

        /**
         * \brief Classifies a channel reference connector for the DAQ.
         */
        enum class ChannelRef
        {
            None               =      0, ///< No reference. Will cause an error if used.
            Default            =     -1, ///< Let DAQmx use the default reference for the current channel.
            SingleEnded        =  10083, ///< Use referenced single ended signal.
            NoRefSingleEnded   =  10078, ///< Use unreferenced single ended signal.
            Differential       =  10106, ///< Use differential signal.
            PseudoDifferential =  12529  ///< Use pseudo-differential signal. See <a href="https://knowledge.ni.com/KnowledgeArticleDetails?id=kA00Z0000019YuUSAU&l=pt-PT">here</a> for more info.
        };

        /**
         * \brief Channel measurement numeric limits.
         * 
         * Only valid for voltage, temperature, etc. type channels.
         * Range - [-10.0, 10.0]
         */
        struct DCS_API ChannelLimits
        {
            DCS::f64 min = -10.0; ///< Minimum DAQ accepted value.
            DCS::f64 max =  10.0; ///< Maximum DAQ accepted value.
        };

        /**
         * \brief Struct used to transfer all the relevant task settings to the server on task creation.
         */
        struct DCS_API TaskSettings
        {
            DCS::Utils::BasicString task_name = { "" };          ///< The name to give to the task.
            DCS::Utils::BasicString channel_name[5] = { "" };    ///< The channels the task reads/writes from/to. Max 5.

            // NOTE : A task only supports channels of the same type (not true, but close enough for this application).
            ChannelType   channel_type   = ChannelType::None;    ///< The type of the channels.
            ChannelRef    channel_ref[5] = { ChannelRef::None }; ///< The reference type for each channel. Max 5.
            ChannelLimits channel_lim[5];                        ///< The limit for each channel, if applicable. Max 5.

            DCS::Utils::BasicString clock = { "OnBoardClock" };  ///< The clock the task will be timed upon, if applicable.
            DCS::f64 clock_rate = 10000;                         ///< The clock rate to be set.
        };

        /**
		 * \brief Create a new DAQ task on the server-side.
		 * 
		 * Registers and configures a task in the DAQmx system.
		 * 
		 * \param setup The options of the task to create.
         * \return Task opaque id handle.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(DCS::DAQ::Task, DCS::DAQ::TaskSettings)
        DCS_API Task NewTask(DCS::DAQ::TaskSettings setup);

        /**
		 * \brief Starts a DAQ task on the server-side by id.
         * 
         * This function marks the start of acquisition.
		 * 
		 * \param task The task id.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(void, DCS::DAQ::Task)
        DCS_API void StartTask(Task task);

        /**
		 * \brief Starts a DAQ task on the server-side by name.
		 * 
         * This function marks the start of acquisition.
         * 
		 * \param task_name The task name.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void StartNamedTask(DCS::Utils::BasicString task_name);

        /**
		 * \brief Stops a DAQ task on the server-side by id.
		 * 
         * This function marks the end of acquisition, if not automatically finished by the server already.
         * 
		 * \param task The task id.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(void, DCS::DAQ::Task)
        DCS_API void StopTask(Task task);


        /**
		 * \brief Stops a DAQ task on the server-side by name.
		 * 
         * This function marks the end of acquisition, if not automatically finished by the server already.
         * 
		 * \param task_name The task name.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void StopNamedTask(DCS::Utils::BasicString task_name);

        /**
		 * \brief Clears a DAQ task on the server-side by id.
		 * 
         * Created functions must always be destroyed.
         * 
		 * \param task The task id.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(void, DCS::DAQ::Task)
        DCS_API void DestroyTask(Task task);

        /**
		 * \brief Clears a DAQ task on the server-side by name.
		 * 
         * Created functions must always be destroyed.
         * 
		 * \param task_name The task name.
		 * 
		 * \ingroup calls
		 */
        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void DestroyNamedTask(DCS::Utils::BasicString task_name);
    }
}

#endif _DCS_ACQ_H