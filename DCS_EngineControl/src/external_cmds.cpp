#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"

void DCS::Control::IssueGenericCommand(UnitTarget target, value_str_test full_command)
{
	static_cast<void>(Coms::GetCmdBuffer().schedule(Coms::Command::Custom(target, full_command.buffer, false)));
}

DCS::Utils::String DCS::Control::IssueGenericCommandResponse(UnitTarget target, const char* full_command)
{
	// DCS::Utils::String implicit ctor
	return Coms::GetCmdBuffer().schedule(Coms::Command::Custom(target, full_command, true)).c_str();
}
