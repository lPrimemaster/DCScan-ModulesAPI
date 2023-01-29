#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Utils/include/internal.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	DCS::Utils::Logger::Init(DCS::Utils::Logger::Verbosity::DEBUG);
	
	LOG_DEBUG("Testing output to console and file.");
	LOG_MESSAGE("Testing output to console and file.");
	LOG_WARNING("Testing output to console and file.");
	LOG_ERROR("Testing output to console and file.");
	LOG_CRITICAL("Testing output to console and file.");

	DCS::Utils::Logger::Destroy();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
