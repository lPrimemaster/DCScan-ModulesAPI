#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../include/DCS_Assert.h"
#include <chrono>

int test()
{
	DCS_START_TEST;
	using namespace DCS::Timer;
	using namespace std::chrono_literals;

	auto timer = New();

	auto a = std::chrono::steady_clock::now();
	while ((std::chrono::steady_clock::now() - a) < 100ms) continue;

	auto ts = GetTimestamp(timer);

	Delete(timer);

	// Ensure ms total precision
	DCS_ASSERT_EQ(ts.millis, 100);

	// Ensure <100us precision
	DCS_ASSERT_LEQ(ts.micros, 100);

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
