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
    namespace DAQ
    {
        typedef DCS::GenericHandle Task;

        ENUM_FLAGS(ChannelType)
        {
            Voltage
        };

        ENUM_FLAGS(ChannelRef)
        {
            Default            =     -1, // DAQmx_Val_Cfg_Default
            SingleEnded        =  10083, // DAQmx_Val_RSE
            NoRefSingleEnded   =  10078, // DAQmx_Val_NRSE
            Differential       =  10106, // DAQmx_Val_Diff
            PseudoDifferential =  12529  // DAQmx_Val_PseudoDiff
        };
    }
}

#endif _DCS_ACQ_H