#include "../../DCS_Core/include/internal.h"
#include "../include/DCS_Assert.h"
#include <chrono>

using namespace std::chrono_literals;

void worker0(std::mutex* lock, std::condition_variable* signal, std::array<std::atomic_int, 16>* flags)
{
	flags->at(0).store(0);

	std::this_thread::sleep_for(200ms);

	DCS::Utils::Logger::Debug("Sample worker0 thread spawned");
	std::unique_lock<std::mutex> l(*lock);

	std::this_thread::sleep_for(1s);

	DCS::Utils::Logger::Debug("Sample worker0 thread flag data as 2.");
	flags->at(0).store(2);

	l.unlock();
	signal->notify_all();


	std::this_thread::sleep_for(1s);

	l.lock();

	DCS::Utils::Logger::Debug("Sample worker0 thread flag data as 1.");
	flags->at(0).store(1);

	l.unlock();
	signal->notify_all();
}

void worker1(std::mutex* lock, std::condition_variable* signal, std::array<std::atomic_int, 16>* flags)
{
	DCS::Utils::Logger::Debug("Sample worker1 thread spawned");

	std::unique_lock<std::mutex> l(*lock);

	DCS::Utils::Logger::Debug("Sample worker1 waiting for data.");

	signal->wait(l, [&] { return flags->at(0).load() == 1; });

	DCS::Utils::Logger::Debug("Sample worker1 received data.");

}

int test()
{
	DCS_START_TEST;

	using namespace DCS::Threading;

	TPool* pool = CreatePersistentThread(2, { worker0, worker1 });

	JoinPool(pool);

	DCS_RETURN_TEST;
}

int main()
{
	return test();
	//return 0;
}