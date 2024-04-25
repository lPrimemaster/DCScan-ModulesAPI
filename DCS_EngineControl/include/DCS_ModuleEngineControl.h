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
 * \version 1.0
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
			ESP301,  ///< The ESP301 rotation stage controller target.
			PMC8742, ///< The PMC8742 tilt stage controller target.
			XPSRLD4  ///< The XPS-RLD4 rotation stage controller target.
		};

		/**
		 * \brief Starts the COM and USB port services.
		 * 
		 * This creates a thread responsible for sending commands for all the targets.
		 * \param esp301_com  The com port (or virtual com port) connecting the pc and the ESP301. Default = "COM3".
		 * \param pmc8742_usb The usb vid/pid of the PMC8742 controller. Default = "104D-4000".
		 * \param xps_rld4    The socket ip for the XPSRLD4 controller. Default = "10.80.0.100".
		 * 
		 * \todo Create a thread for each target. Allowing for simultaneous operation of the stages.
		 */
		DCS_API void StartServices(const char* esp301_com = "COM3", const char* pmc8742_usb = "104D-4000", const char* xps_rld4 = "10.80.0.100");

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
		 * 
		 * \ingroup calls
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
		 * 
		 * \ingroup calls
		 */
		DCS_REGISTER_CALL(DCS::Utils::BasicString, DCS::Control::UnitTarget, DCS::Utils::BasicString)
		DCS_API DCS::Utils::BasicString IssueGenericCommandResponse(UnitTarget target, DCS::Utils::BasicString full_command);

		/**
		 * \brief Enumerates the diferent status of the mannual PID.
		 */
		enum class PIDStatus
		{
			OFF,	///< Manual PID control is disabled.
			READY,	///< Manual PID control is ready to work.
			WORKING ///< Manual PID control is busy working.
		};

		/**
		 * \brief Holds the PIDStatus data for a given target-group configuration.
		 */
		struct PIDStatusGroup
		{
			UnitTarget target;   		   ///< The configuration's target.
			DCS::Utils::BasicString group; ///< The configuration's group.
			PIDStatus status;			   ///< The configuration's status.
		};

		/**
		 * \brief Set the gain parameters for the controller PID and associate an encoder axis readout.
		 * 
		 * This requires an encoder to be connected to the engine. If the encoder is already present on the assembly, two options arise
		 * Readout of the encoder value associated to using the custom made controller functions.
		 * Readout of the encoder value associated to using internal closed loop controller funtions.
		 *
		 * \param target The stage to target.
		 * \param group The name of the positioner for the XPS-RLD controller, or the axis number for the ESP301-G.
		 * \param encoder_axis The number of the encoders' axis that is to controll this motion correction algorithm. Pass -1 to this field if you wish
		 * to use the internal encoder directly from the ESP301-G or the XPS-RLD controllers.
		 * \param Kp The constant gain of the PID.
		 * \param Ki The integral gain of the PID.
		 * \param Ki The differential gain of the PID.
		 * 
		 * 
		 * \ingroup calls
		 */
		DCS_REGISTER_CALL(void, DCS::Control::UnitTarget, DCS::Utils::BasicString, DCS::i8, DCS::f64, DCS::f64, DCS::f64)
		DCS_API void SetPIDControlVariables(UnitTarget target, DCS::Utils::BasicString group, i8 encoder_axis, f64 Kp, f64 Ki, f64 Kd);

		/**
		 * \brief Move the specified positioner group / axis according to the parameters set for the PID.
		 *
		 * Moves a positioner / axis according to a custom made internal PID using the control gains set via the SetPIDControlVariables function.
		 * 
		 * \todo Make this work with the ESP301-G.
		 *
		 * \param target The stage to target.
		 * \param group The name of the positioner for the XPS-RLD controller, or the axis number for the ESP301-G.
		 * \param target_position The target absolution position to move to. 
		 * 
		 * 
		 * \ingroup calls
		 */
		DCS_REGISTER_CALL(void, DCS::Control::UnitTarget, DCS::Utils::BasicString, DCS::f64)
		DCS_API void MoveAbsolutePID(UnitTarget target, DCS::Utils::BasicString group, f64 target_position);

		/**
		 * \brief Move the specified positioner group / axis relative to the current position.
		 *
		 * Moves a positioner / axis arelative to the current position.
		 * 
		 *
		 * \param target The stage to target.
		 * \param group The name of the positioner for the XPS-RLD controller, or the axis number for the ESP301-G.
		 * \param target_position The target relative position to move to. 
		 * 
		 * 
		 * \ingroup calls
		 */
		DCS_REGISTER_CALL(void, DCS::Control::UnitTarget, DCS::Utils::BasicString, DCS::f64)
		DCS_API void MoveRelative(UnitTarget target, DCS::Utils::BasicString group, f64 target_position);

		/**
		 * \brief Gets called when the move absolute pid controller status changes.
		 * 
		 * \param status_group The configuration-group and its status.
		 * 
		 * \ingroup events
		 */
		DCS_REGISTER_EVENT
		DCS_API void MoveAbsolutePIDChanged(PIDStatusGroup status_group);
	}
}

#endif _DCS_ENGINECONTROL_H
