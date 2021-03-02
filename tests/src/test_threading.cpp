#include "../../DCS_Core/include/internal.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/DCS_Assert.h"
#include <chrono>

using namespace std::chrono_literals;

void worker0(std::mutex* lock, std::condition_variable* signal, std::array<std::atomic_int, 16>* flags)
{
	flags->at(0).store(0);

	std::this_thread::sleep_for(100ms);

	LOG_DEBUG("Sample worker0 thread spawned");
	std::unique_lock<std::mutex> l(*lock);

	std::this_thread::sleep_for(100ms);

	LOG_DEBUG("Sample worker0 thread flag data as 2.");
	flags->at(0).store(2);

	l.unlock();
	signal->notify_all();


	std::this_thread::sleep_for(100ms);

	l.lock();

	LOG_DEBUG("Sample worker0 thread flag data as 1.");
	flags->at(0).store(1);

	l.unlock();
	signal->notify_all();
}

void worker1(std::mutex* lock, std::condition_variable* signal, std::array<std::atomic_int, 16>* flags)
{
	LOG_DEBUG("Sample worker1 thread spawned");

	std::unique_lock<std::mutex> l(*lock);

	LOG_DEBUG("Sample worker1 waiting for data.");

	signal->wait(l, [&] { return flags->at(0).load() == 1; });

	LOG_DEBUG("Sample worker1 received data.");
}

int test()
{
	DCS_START_TEST;

	using namespace DCS::Threading;

	TPool* pool = CreatePersistentPool(2, { worker0, worker1 });

	JoinPool(pool);

	DCS_ASSERT_EQ(pool->flags.at(0).load(), 1);

	DestroyPool(pool);

	TPool* pool_large_fail = CreatePersistentPool(GetMaxHardwareConcurrency() + 1, 
		{ worker0, worker1, worker1, worker1, worker1, worker1, worker1, worker1, 
		worker1, worker1, worker1, worker1, worker1, worker1, worker1, worker1,
		worker1, worker1, worker1, worker1 });

	DCS_ASSERT_EQ(GetPoolWorkCount(pool_large_fail), GetMaxHardwareConcurrency());

	JoinPool(pool_large_fail);

	DCS_ASSERT_EQ(pool_large_fail->flags.at(0).load(), 1);

	DestroyPool(pool_large_fail);

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}