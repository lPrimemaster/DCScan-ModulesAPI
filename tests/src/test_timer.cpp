#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../include/DCS_Assert.h"
#include <chrono>

int test()
{
	using namespace DCS::Timer;
	using namespace std::chrono_literals;

	auto timer = New();

	auto a = std::chrono::steady_clock::now();
	while ((std::chrono::steady_clock::now() - a) < 100ms) continue;

	auto ts = GetTimestamp(timer);

	Delete(timer);

	std::cout << "Expected 100ms... => Got " << ts.millis << "ms" << std::endl;
	std::cout << "Total => " << ts.to_string().c_str() << std::endl;

	// Ensure <5us precision
	return DCS_ASSERT_EQ(ts.millis, 100) && DCS_ASSERT_LEQ(ts.micros, 5);
}

int main()
{
	return test();
}
