#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"

void DCS::Control::IssueGenericCommand(UnitTarget target, DCS::Utils::BasicString full_command)
{
	static_cast<void>(Coms::GetCmdBuffer().schedule(Coms::Command::Custom(target, full_command.buffer, false)));
}

DCS::Utils::BasicString DCS::Control::IssueGenericCommandResponse(UnitTarget target, DCS::Utils::BasicString full_command)
{
	DCS::Utils::BasicString ret;
	const char* val = Coms::GetCmdBuffer().schedule(Coms::Command::Custom(target, full_command.buffer, true)).c_str();
	memcpy(ret.buffer, val, strlen(val));
	return ret;
}
