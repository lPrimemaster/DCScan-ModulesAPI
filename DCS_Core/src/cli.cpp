#include "../include/DCS_ModuleCore.h"

static DCS::i32 LevenshteinDistance(std::string s1, std::string s2)
{
	using namespace DCS;

	i32 L1 = s1.size() + 1;
	i32 L2 = s2.size() + 1;

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
	i32 min = 8;
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

void DCS::CLI::WaitForCommands()
{
#pragma warning( push )
#pragma warning( disable : 26444 )
	Command("help", "Displays this help message.", []() { LOG_DEBUG("Not very helpful..."); });
	Command("stop", "Stops the server execution.");
#pragma warning( pop )

	std::string cmd_str;
	Command* cmd = nullptr;
	while (true)
	{
		if (std::cin >> cmd_str, cmd = Command::Get(cmd_str), cmd != nullptr)
		{
			cmd->Run();
		}
		else
		{
			LOG_MESSAGE("Could not find command %s.", cmd_str.c_str());
			LOG_MESSAGE("Did you mean %s.", Command::Closest(cmd_str)->getName().c_str());
		}
	}
}