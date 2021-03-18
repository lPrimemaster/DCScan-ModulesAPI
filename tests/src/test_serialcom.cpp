#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_EngineControl/include/internal.h"

#include "../../DCS_Network/include/DCS_ModuleNetwork.h"

#include "../include/DCS_Assert.h"

int main()
{
	DCS_START_TEST;

	using namespace DCS::Control;
	using namespace DCS::Network;

	char buff[1024];
	DCS::i32 size = DCS::Serial::enumerate_ports(buff, 1024);

	char* w = std::strtok(buff, "\0");
	LOG_DEBUG("Port0: %s", w);
	LOG_DEBUG("Size: %d", size);

	StartServices();

	// Set the ESP301 axis 1 to 90.5 deg and wait for stop. After, get position.
	//auto r = IssueGenericCommandResponse(UnitTarget::PMC8742, "1PA90.50;WS;1TP?");

	//value_str_test str = { "2MO;2PA45.0;2WS;2PA0.00;2WS;2MF" };
	//str.buffer = "2MO;2PA45.0;2WS;2PA0.00;2WS;2MF";

	std::this_thread::sleep_for(std::chrono::seconds(1));

	IssueGenericCommand(UnitTarget::ESP301, { "2MO;2PA5.00;2WS;2PA0.00;2WS;2MF"  });

	Init();

	auto client = Server::WaitForConnection(Server::Create(15777));

	Server::StartThread(client);

	StartServices();

	// Set the ESP301 axis 1 to 90.5 deg and wait for stop. After, get position.
	//auto r = IssueGenericCommandResponse(UnitTarget::PMC8742, "1PA90.50;WS;1TP?");

	//value_str_test str = { "2MO;2PA45.0;2WS;2PA0.00;2WS;2MF" };
	//str.buffer = "2MO;2PA45.0;2WS;2PA0.00;2WS;2MF";

	//LOG_DEBUG("ESP301 [axis 1] stopped at: %s", val.c_str());

	std::this_thread::sleep_for(std::chrono::seconds(3600));

	Server::StopThread(client);

	Destroy();

	StopServices();

	DCS_RETURN_TEST;
}