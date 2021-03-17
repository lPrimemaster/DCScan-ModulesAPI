#ifndef _DCS_ENGINECONTROL_H
#define _DCS_ENGINECONTROL_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

/**
 * @file
 * \brief Provide an example
 *
 * This class is meant as an example. It is not useful by itself
 * rather its usefulness is only a function of how much it helps
 * the reader.  It is in a sense defined by the person who reads it
 * and otherwise does not exist in any real form.
 *
 * \author César Godinho
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
		 * \brief Enumerates the diferent devices acessible via COM ports.
		 * 
		 * Value of components is for identification only. It is not related to the OS COM port id.
		 */
		enum class DCS_API UnitTarget
		{
			ESP301, ///< The ESP301 rotation stage controller target.
			PMC8742 ///< The PMC8742 tilt stage controller target.
		};

		/**
		 * \brief Starts the COM port services.
		 * 
		 * This creates a thread responsible for sending commands for all the targets.
		 * 
		 * \todo Create a thread for each target. Allowing for simultaneous operation of the stages.
		 */
		DCS_API void StartServices();

		/**
		 * \brief Stops the COM port services.
		 *
		 * This destroys all the context of COM handles and disables remote connection.
		 *
		 * \todo Clean up motors to a reset state as well.
		 */
		DCS_API void StopServices();

		struct value_str_test
		{
			char buffer[512];
		};

		/**
		 * \brief Issue a generic command to the specified target.
		 * 
		 * This function sends a command to a controller using its commands.
		 * See the ESP301-3G and PMC8742 controller manuals for the raw commands to send via this function.
		 * 
		 * Separate commands for the ESP301-3G and PMC8742 controller's using semicolons.
		 * 
		 * \param target The stage to target.
		 * \param full_command The ASCII command to send to the unit.
		 */
		DCS_REGISTER_CALL(void, DCS::Control::UnitTarget, DCS::Control::value_str_test)
		DCS_API void IssueGenericCommand(UnitTarget target, value_str_test full_command);

		/**
		 * \brief Issue a generic command to the specified target, waiting for a response.
		 *
		 * This function sends a command to a controller using its commands.
		 * See the ESP301-3G and PMC8742 controller manuals for the raw commands to send via this function.
		 * 
		 * Separate commands for the ESP301-3G and PMC8742 controller's using semicolons.
		 * 
		 * The part of the command responsible for requesting data can be anywhere in the command chain.
		 *
		 * \param target The stage to target.
		 * \param full_command The ASCII command to send to the unit.
		 * 
		 * \return A string containing the raw target controller's response.
		 */
		DCS_API DCS::Utils::String IssueGenericCommandResponse(UnitTarget target, const char* full_command);
	}
}

#endif _DCS_ENGINECONTROL_H
