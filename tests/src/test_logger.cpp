#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	DCS::Utils::Logger::Init(DCS::Utils::Logger::Verbosity::DEBUG);
	
	DCS::Utils::Logger::Debug("Testing output to console and file.");
	DCS::Utils::Logger::Message("Testing output to console and file.");
	DCS::Utils::Logger::Warning("Testing output to console and file.");
	DCS::Utils::Logger::Error("Testing output to console and file.");
	DCS::Utils::Logger::Critical("Testing output to console and file.");

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
