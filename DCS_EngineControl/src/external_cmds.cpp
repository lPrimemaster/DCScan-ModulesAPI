#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"
#include "../../DCS_Core/include/internal.h"
#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"

#include <unordered_map>

static std::unordered_map<std::string, DCS::PositionCorrector::PIDParams> pid_targets;


DCS::PositionCorrector::PIDParams::PIDParams(f64 Kp, f64 Ki, f64 Kd, f64 min, f64 max, f64 bias, i8 ax) : pid(min, max, Kp, Kd, Ki), ax(ax) {  }

void DCS::Control::IssueGenericCommand(UnitTarget target, DCS::Utils::BasicString full_command)
{
	static_cast<void>(Coms::GetCmdBuffer().schedule(Coms::Command::Custom(target, full_command.buffer, false)));
}

DCS::Utils::BasicString DCS::Control::IssueGenericCommandResponse(UnitTarget target, DCS::Utils::BasicString full_command)
{
	DCS::Utils::BasicString ret;
	auto str = Coms::GetCmdBuffer().schedule(Coms::Command::Custom(target, full_command.buffer, true));
	memcpy(ret.buffer, str.c_str(), str.size() + 1);
	return ret;
}

void DCS::Control::SetPIDControlVariables(UnitTarget target, DCS::Utils::BasicString group, i8 encoder_axis, f64 Kp, f64 Ki, f64 Kd)
{
	// Just overwrite the old pid system. This will delete the integral term, but maybe that can be
	// a plus when testing some values for the gains.
	pid_targets.insert_or_assign(std::to_string(static_cast<int>(target)) + group.buffer, PositionCorrector::PIDParams(Kp, Ki, Kd, -50.0, 50.0, 0.0, encoder_axis));


	// TODO: Check if the encoder_axis is available (if != -1)
	// TODO: Check if the target+group is available
}

DCS::f64 DCS::Control::MoveAbsolutePID(UnitTarget target, DCS::Utils::BasicString group, f64 target_position)
{
	std::string target_id = std::to_string(static_cast<int>(target)) + group.buffer;
	auto pid_target_it = pid_targets.find(target_id);
	if(pid_target_it == pid_targets.end())
	{
		LOG_ERROR("Could not find a target/group combo with registered PID variables.");
		LOG_ERROR("Maybe forgot to call SetPIDControlVariables()?.");
		return 0.0;
	}

	auto& pid_target = pid_target_it->second;

	// Lets test this and use a target min error to iterate to until finish
	// Use a SA just to check, ignore the PID!
	
	if(pid_target.ax == -1)
	{
		// TODO
	}

	f64 pos = ENC::InspectLastEncoderValues().axis[pid_target.ax-1].calpos;
	f64 error = 1.0;
	while(std::abs(error) > 0.00005)
	{
		pid_target.pid.setTargetAndBias(target_position, 0.0);
		f64 relative_move = pid_target.pid.calculate(pos);

		// TODO: std::to_string() only accounts for 6 decimal places, this will be an issue
		DCS::Utils::BasicString relative_move_command;
		std::string cmd = std::string("GroupMoveRelative(" + std::string(group.buffer) + ".Pos," + std::to_string(relative_move) + ")");
		memcpy(relative_move_command.buffer, cmd.c_str(), cmd.size() + 1);

		IssueGenericCommandResponse(target, relative_move_command);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(150));

		pos = ENC::InspectLastEncoderValues().axis[pid_target.ax-1].calpos;
		error = target_position - pos;

		LOG_DEBUG("Target: %lf", target_position);
		LOG_DEBUG("Position: %lf", pos);
		LOG_DEBUG("Error: %lf", error);
	}

	return error;
}
