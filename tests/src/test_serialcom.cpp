#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_EngineControl/include/internal.h"
#include "../include/DCS_Assert.h"

int main()
{
	DCS_START_TEST;

	using namespace DCS::Control;

	char buff[1024];
	DCS::i32 size = DCS::Serial::enumerate_ports(buff, 1024);

	char* w = std::strtok(buff, "\0");
	LOG_DEBUG("Port0: %s", w);

	StartServices();

	// Set the ESP301 axis 1 to 90.5 deg and wait for stop. After, get position.
	//auto r = IssueGenericCommandResponse(UnitTarget::PMC8742, "1PA90.50;WS;1TP?");
	IssueGenericCommand(UnitTarget::PMC8742, "1PA90.50;WS;1TP?");

	//LOG_DEBUG("ESP301 [axis 1] stopped at: %s", r.c_str());

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	StopServices();

	DCS_RETURN_TEST;
}