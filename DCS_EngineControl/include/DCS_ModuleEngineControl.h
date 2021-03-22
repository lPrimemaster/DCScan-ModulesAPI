#ifndef _DCS_ENGINECONTROL_H
#define _DCS_ENGINECONTROL_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

/**
 * @file
 * \brief Exposes engine control functionalities of the API to the end user.
 *
 * \author Cesar Godinho
 *
 * \version 1.0 $Revision: 1.5 $
 *
 * \date $Date: 2020/10/12$
 */

namespace DCS
{
	/**
	 * \brief Exposes tilt and rotation stages control parameters.
	 */
	namespace Control
	{
		/**
		 * \brief Enumerates the diferent devices acessible via COM/USB ports.
		 * 
		 * Value of components is for identification only. It is not related to the OS COM/USB port id.
		 */
		enum class UnitTarget
		{
			ESP301, ///< The ESP301 rotation stage controller target.
			PMC8742 ///< The PMC8742 tilt stage controller target.
		};

		/**
		 * \brief Starts the COM and USB port services.
		 * 
		 * This creates a thread responsible for sending commands for all the targets.
		 * 
		 * \todo Create a thread for each target. Allowing for simultaneous operation of the stages.
		 */
		DCS_API void StartServices();

		/**
		 * \brief Stops the COM and USB port services.
		 *
		 * This destroys all the context of COM and USB handles and disables remote connection.
		 *
		 * \todo Clean up motors to a reset state as well.
		 */
		DCS_API void StopServices();

		/**
		 * \brief Issue a generic command to the specified target.
		 * 
		 * This function sends a command to a controller using its commands.
		 * See the ESP301-3G and PMC8742 controller manuals for the raw commands to send via this function.
		 * 
		 * Separate commands for the ESP301-3G controller's using semicolons.
		 * The PMC8742 only supports non separated commands.
		 * \todo Create a wrapper to emulate PMC8742 command separator via (;).
		 * 
		 * \param target The stage to target.
		 * \param full_command The ASCII command to send to the unit.
		 */
		DCS_REGISTER_CALL(void, DCS::Control::UnitTarget, DCS::Utils::BasicString)
		DCS_API void IssueGenericCommand(UnitTarget target, DCS::Utils::BasicString full_command);

		/**
		 * \brief Issue a generic command to the specified target, waiting for a response.
		 *
		 * This function sends a command to a controller using its commands.
		 * See the ESP301-3G and PMC8742 controller manuals for the raw commands to send via this function.
		 * 
		 * Separate commands for the ESP301-3G controller's using semicolons.
		 * The PMC8742 only supports non separated commands.
		 * \todo Create a wrapper to emulate PMC8742 command separator via (;).
		 * 
		 * The part of the command responsible for requesting data can be anywhere in the command chain.
		 *
		 * \param target The stage to target.
		 * \param full_command The ASCII command to send to the unit.
		 * 
		 * \return A string containing the raw target controller's response.
		 */
		DCS_REGISTER_CALL(DCS::Utils::BasicString, DCS::Control::UnitTarget, DCS::Utils::BasicString)
		DCS_API DCS::Utils::BasicString IssueGenericCommandResponse(UnitTarget target, DCS::Utils::BasicString full_command);
	}
}

#endif _DCS_ENGINECONTROL_H
