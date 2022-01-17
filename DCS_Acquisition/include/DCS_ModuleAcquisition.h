#ifndef _DCS_ACQ_H
#define _DCS_ACQ_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"


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
         * \brief Holds all DCS data relative to a count event.
         */
        struct DCS_API DCSCountEventData
        {
            DCS::u64 counts;                 ///< The total X-ray unique counts.
            DCS::Timer::Timestamp timestamp; ///< The time these counts were performed (software time).
            DCS::f64 expected_angle;         ///< The expected angle of the engine at time of counts (x = vt).
            DCS::f64 measured_angle;         ///< The measured angle of the engine at time of counts.
        };

        /**
         * \brief Holds all MCA data relative to a count event.
         */
        struct DCS_API MCACountEventData
        {
            DCS::u64 count;                  ///< Stores `bins` array size in words (16-bit windows like for 64-bit machines).
            DCS::Timer::Timestamp timestamp; ///< The time these counts were performed (software time).
            DCS::u16 total_max_bin_count;    ///< Max size of channels in the MCA (typically 1024 / 2048 / ... / 65535 (max for NI-6229))
            DCS::u16 bins[128];              ///< The bins value array (this is not a frequency list).
        };

        /**
         * \brief Initializes the Acquisition API.
         */
        DCS_API void Init();

        /**
         * \brief Cleans up the Acquisition API.
         */
        DCS_API void Terminate();

        /**
         * \brief Creates a new voltage channel to add to the acquisition stage.
         * 
         * Adding multiple channels will cause the selected acquisition sample rate (Selected in StartAIAcquisition) 
         * to be divided by all channels. E.g. create two new channels via DCS::DAQ::NewAIVChannel with a total of 1000 S/s will
         * retsult in a total of 500 S/s per channel being offseted in the SV_EVT_DCS_DAQ_VoltageEvent event.
         * 
         * The new added channel will only take effect when a new acquisition is started via StartAIAcquisition.
         * 
         * \param name The frindly name to give to this channel.
         * \param channel_name The hardware name of the channel in the DAQ (e.g. PXI_slot2/ai0).
         * \param ref The channel reference settings. Refer to the ChannelRef enum.
         * \param lim The channel voltage limits. Refer to the ChannelLimits enum.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(void, DCS::Utils::BasicString, DCS::Utils::BasicString, DCS::DAQ::ChannelRef, DCS::DAQ::ChannelLimits)
        DCS_API void NewAIVChannel(DCS::Utils::BasicString name, DCS::Utils::BasicString channel_name, ChannelRef ref, ChannelLimits lim);

        /**
         * \brief Deletes an existing voltage channel from the acquisition stage.
         * 
         * The new deleted channel will only take effect when a new acquisition is started via StartAIAcquisition.
         * 
         * \param name The friendly name given to this channel.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(void, DCS::Utils::BasicString)
        DCS_API void DeleteAIVChannel(DCS::Utils::BasicString name);

        /**
         * \brief Starts the AI (Analog Input) data acquisition.
         * 
         * All the added channels will start to acquire its value to the SV_EVT_DCS_DAQ_VoltageEvent event. 
         * All other events related to this, such as the main engine-detector setup for energy analisys will also
         * start to acquire when this function is called.
         * 
         * \param samplerate The sample rate to use for all channels combined.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(void, DCS::f64)
        DCS_API void StartAIAcquisition(DCS::f64 samplerate);

        /**
         * \brief Stops the AI (Analog Input) data acquisition.
         * 
         * All the added channels will stop from acquiring its value to the SV_EVT_DCS_DAQ_VoltageEvent event. 
         * All other events related to this, such as the main engine-detector setup for energy analisys will also
         * stop from acquiring when this function is called.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(void)
        DCS_API void StopAIAcquisition();


        /**
         * \brief Retreives the number of channels outputing from the MCA event counter.
         * Defaults to 2048.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(DCS::u16)
        DCS_API u16 GetMCANumChannels();

        /**
         * \brief Retreives the number of channels outputing from the MCA event counter.
         * Defaults to 2048.
         * 
         * \param nChannels The number of channels to attribute. Must be smaller than INTERNAL_ADC_MAX_CHAN.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(void, DCS::u16)
        DCS_API void SetMCANumChannels(u16 nChannels);

        /**
         * \brief Returns the maximum clock speed allowed by the ADC.
         * 
         * \param nChannels The number of channels to attribute. Must be smaller than INTERNAL_ADC_MAX_CHAN.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(DCS::f64)
        DCS_API f64 GetADCMaxInternalClock();

        // TODO : Document
        DCS_REGISTER_EVENT
        DCS_API void DCSCountEvent();

        // TODO : Document
        DCS_REGISTER_EVENT
        DCS_API void MCACountEvent(u16 mca_num_channels);
    }

    /**
     * \brief Exposes %ENC functionalities of the API to the end user.
     */
    namespace ENC
    {
        /* 
         *  \brief Struct for encoder soft realtime mode data (per axis).
         */
        struct DCS_API EncoderAxisData
        {
            i8  axis;            //< Axis value
            i64 position;        //< Position value
            f64 calpos;          //< Double converted position
            u16 status;          //< Status word
            u16 triggerCounter;  //< Trigger counter value
            u32 timestamp;       //< Timestamp
            i64 ref[2];          //< Reference position values
        };

        /* 
         *  \brief Struct for encoder soft realtime mode data (all axes).
         */
        struct DCS_API EncoderData
        {
            i8 numAxis;              //< Number of axis present;
            EncoderAxisData axis[4]; //< Encoder axes list
        };

        /**
         * \brief Returns the last value output from the angular encoder.
         * 
         * Returns EncoderData with zero numAxis if failed.
         * 
         * \ingroup calls
         */
        DCS_REGISTER_CALL(DCS::ENC::EncoderData)
        DCS_API EncoderData InspectLastEncoderValues();

        /**
         * \brief Initializes the rotary encoder API.
         * \param ip The rotary encoder LAN ip address.
         * \param axis The axis to enable. Bit flags type: b0 - axis1, b1 - axis2, ... => e.g. 0b0101 enables axis 1 and 3 
         */
        DCS_API void Init(const char* ip, i8 axis);

        /**
         * \brief Cleans up the rotary encoder API.
         */
        DCS_API void Terminate();
    }
}

#endif _DCS_ACQ_H