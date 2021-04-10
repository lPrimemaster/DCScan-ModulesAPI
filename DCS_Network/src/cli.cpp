#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"

static DCS::Timer::SystemTimer cli_uptime;

static DCS::i32 LevenshteinDistance(std::string s1, std::string s2)
{
	using namespace DCS;

	i32 L1 = (i32)s1.size() + 1;
	i32 L2 = (i32)s2.size() + 1;

	i32* DM = (i32*)malloc((i64)L1 * (i64)L2 * sizeof(i32));

	auto IX = [&](int i, int j) -> i32& { return DM[i * L2 + j]; };

	for (int i = 0; i < L1; i++)
	{
		IX(i, 0) = i;
	}
	for (int j = 0; j < L2; j++)
	{
		IX(0, j) = j;
	}

	for (int j = 1; j < L2; j++)
	{
		for (int i = 1; i < L1; i++)
		{
			i32 cost = s1[i] != s2[i] ? 1 : 0;
			i32 cI = IX(i - 1, j) + 1;
			i32 cD = IX(i, j - 1) + 1;
			i32 cS = IX(i - 1, j - 1) + cost;

			if (cI <= cD)
			{
				if (cI <= cS)
					IX(i, j) = cI;
				else
					IX(i, j) = cS;
			}
			else
			{
				if (cD <= cS)
					IX(i, j) = cD;
				else
					IX(i, j) = cS;
			}
		}
	}

	i32 Ldist = IX(L1 - 1, L2 - 1);
	free(DM);

	return Ldist;
}

DCS::CLI::Command* DCS::CLI::Command::Closest(std::string name)
{
	i32 min = 0xFFFFFF;
	Command* min_c = nullptr;

	for (auto& p : cmd_reg)
	{
		std::string cmd = p.second.cmd_name;
		i32 lmin = LevenshteinDistance(name, cmd);

		if (lmin < min)
		{
			min = lmin;
			min_c = &p.second;
		}
	}

	return min_c;
}

static void CommandRegistry()
{
	using namespace DCS::CLI;
#pragma warning( push )
#pragma warning( disable : 26444 )

	Command("help", "Displays this help message.", [](bool* brk) {
		LOG_MESSAGE("There you go!");
		for (auto c : Command::ListCommands())
		{
			LOG_MESSAGE("%s", c.c_str());
		}
	});

	Command("stop", "Stops the server execution.", [](bool* brk) {
		if (DCS::Network::Server::IsRunning())
		{
			if((SOCKET)DCS::Network::Server::GetConnectedClient() != INVALID_SOCKET)
			{
				char ip[128];
				DCS::Network::GetSocketIpAddress((SOCKET)DCS::Network::Server::GetConnectedClient(), ip);
				LOG_WARNING("A client is currently connected to the server: [at %s]", ip);
				LOG_WARNING("Stop server anyways? y/n");

				std::string s;
				std::cin >> s;
				if(s[0] == 'y')
				{
					LOG_MESSAGE("Stopping server..."); *brk = true; // The only true use case of the cli brk flag
					DCS::Network::Server::StopThread(DCS::Network::Server::GetConnectedClient(), DCS::Network::Server::StopMode::IMMEDIATE);
				}
				else
				{
					LOG_MESSAGE("Aborting stop...");
				}
			}
		}
		else
		{
			LOG_MESSAGE("Stopping server..."); *brk = true; // The only true use case of the cli brk flag
		}
	});

	Command("uptime", "Prints how long the server (CLI) has been running.", [](bool* brk) {
		LOG_MESSAGE("[%s]", DCS::Timer::GetTimestampStringSimple(cli_uptime).c_str());
	});

	// TODO : Add more CLI commands
	// - Disconnect user
	// - Force stop current task tree
	// - Override remote commands
	// - Suspend -> Pause current task to resume later
	// - Resume

	// TODO : Implement easter eggs commands 
	// - Funny statistics ( # data collected, # time collecting, # deg spun, ... )

#pragma warning( pop )
}

void DCS::CLI::Spin()
{
	cli_uptime = DCS::Timer::New();

	CommandRegistry();

	std::string cmd_str;
	Command* cmd = nullptr;
	bool brk = false; 

	while (!brk)
	{
		if (std::cin >> cmd_str, cmd = Command::Get(cmd_str), cmd != nullptr)
		{
			cmd->Run(&brk);
		}
		else
		{
			LOG_MESSAGE("Could not find command %s.", cmd_str.c_str());
			LOG_MESSAGE("Did you mean \"%s\"?", Command::Closest(cmd_str)->getName().c_str());
			LOG_MESSAGE("Type \"help\" to see the valid commands.");
		}
	}

	DCS::Timer::Delete(cli_uptime);
}
