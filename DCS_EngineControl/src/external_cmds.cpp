#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"
#include "../../DCS_Core/include/internal.h"
#include "../../DCS_Acquisition/include/DCS_ModuleAcquisition.h"
#include "../../DCS_Network/include/internal.h"

#include <unordered_map>

static std::unordered_map<std::string, DCS::PositionCorrector::PIDParams> pid_targets;
static std::unordered_map<std::string, std::atomic<bool>> pid_targets_working;


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
	pid_targets_working.insert_or_assign(std::to_string(static_cast<int>(target)) + group.buffer, false);

	// TODO: Check if the encoder_axis is available (if != -1)
	// TODO: Check if the target+group is available
}

void DCS::Control::MoveAbsolutePID(UnitTarget target, DCS::Utils::BasicString group, f64 target_position)
{
	std::string target_id = std::to_string(static_cast<int>(target)) + group.buffer;
	auto pid_target_it = pid_targets.find(target_id);
	if(pid_target_it == pid_targets.end())
	{
		LOG_ERROR("Could not find a target/group combo with registered PID variables.");
		LOG_ERROR("Maybe forgot to call SetPIDControlVariables()?");
		return;
	}

	auto& pid_target = pid_target_it->second;
	
	if(pid_target.ax == -1)
	{
		LOG_ERROR("Custom PID controller is not yet available for internal encoder motors");
		return;
	}

	// TODO: This needs a tweak for longer span (> .01 deg) movements, since they are less accurate (tend to overshoot)
	// NOTE: There seems to be a drift in motion after the mechanical activation (lasts for about ?? mins)

	if(pid_targets_working[target_id])
	{
		LOG_WARNING("Cannot execute PID motion. A PID motion for the given target is still in progress.");
		return;
	}

	pid_targets_working[target_id] = true;
	MoveAbsolutePIDChanged({target, group, PIDStatus::WORKING});

	std::thread([&pid_target, target_id, target, group, target_position]() -> void {
		f64 pos = ENC::InspectLastEncoderValues().axis[pid_target.ax-1].calpos;
		f64 error = target_position - pos;
		u32 wait_interval = ENC::GetTriggerPeriod() + 50000; // Wait for (in microseconds)

		// Make to passes to make sure we are on the right spot
		for(i32 i = 0; i < 2; i++)
		{
			while(std::abs(error) > 0.00005)
			{
				pid_target.pid.setTargetAndBias(target_position, 0.0);
				f64 relative_move = pid_target.pid.calculate(pos);

				// TODO: std::to_string() only accounts for 6 decimal places, this will be an issue
				DCS::Utils::BasicString relative_move_command;
				std::string cmd = std::string("GroupMoveRelative(" + std::string(group.buffer) + ".Pos," + std::to_string(relative_move) + ")");
				memcpy(relative_move_command.buffer, cmd.c_str(), cmd.size() + 1);

				IssueGenericCommandResponse(target, relative_move_command);
				
				std::this_thread::sleep_for(std::chrono::microseconds(wait_interval));

				pos = ENC::InspectLastEncoderValues().axis[pid_target.ax-1].calpos;
				error = target_position - pos;

				LOG_DEBUG("PID Info - T: %.6lf P: %.6lf E: %.6lf", target_position, pos, error);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			pos = ENC::InspectLastEncoderValues().axis[pid_target.ax-1].calpos;
			error = target_position - pos;
		}

		pid_targets_working[target_id] = false;
		MoveAbsolutePIDChanged({target, group, PIDStatus::READY});
	}).detach();
}

void DCS::Control::MoveAbsolutePIDChanged(PIDStatusGroup status_group)
{	
	DCS_EMIT_EVT((DCS::u8*)&status_group, sizeof(PIDStatusGroup));
}
