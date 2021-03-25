#include "../include/DCS_ModuleCore.h"

void DCS::CLI::WaitForCommands()
{
	Command("help", "Displays this help message.", []() { LOG_DEBUG("Not very helpful..."); });
	Command("stop", "Stops the server execution.");

	std::string cmd_str;
	Command* cmd = nullptr;
	while (true)
	{
		if (std::cin >> cmd_str, cmd = Command::Get(cmd_str), cmd != nullptr)
		{
			LOG_DEBUG("Ahm...");
			cmd->Run();
		}
	}
}
