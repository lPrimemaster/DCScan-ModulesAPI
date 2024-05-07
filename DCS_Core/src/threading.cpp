#include <thread>
#include "../include/DCS_ModuleCore.h"
#include "../include/internal.h"

const DCS::u16 DCS::Threading::GetMaxHardwareConcurrency()
{
	return std::thread::hardware_concurrency();
}

DCS::Threading::TPool* DCS::Threading::CreatePersistentPool(u16 size, std::vector<std::function<void(std::mutex*, std::condition_variable*, std::array<std::atomic_int, 16>*)>> workers)
{
	u16 isize = size;
	
	TPool* pool = new TPool();

	if (size > GetMaxHardwareConcurrency())
	{
		isize = GetMaxHardwareConcurrency();
		LOG_WARNING("Attempt to create a thread_pool with size > max_physical_threads");
		LOG_WARNING("Creating %d threads instead.", isize);
		LOG_CRITICAL("Discarded last %d workers from TPool %x!", workers.size() - isize, pool);
	}

	LOG_DEBUG("Initialized persistent threads. (size=%d)", isize);

	for (u16 i = 0; i < isize; i++)
		pool->workers.push_back(std::thread(workers[i], &pool->lock, &pool->signal, &pool->flags));

	return pool;
}

void DCS::Threading::JoinPool(TPool* pool)
{
	for (std::thread& w : pool->workers)
	{
		w.join();
	}
	pool->workers.clear();
}

DCS::u64 DCS::Threading::GetPoolWorkCount(TPool* pool)
{
	return pool->workers.size();
}

void DCS::Threading::DestroyPool(TPool* pool)
{
	if (!pool->workers.size())
	{
		LOG_DEBUG("Deleted pool object %x.", pool);
		delete pool;
	}
	else
		LOG_WARNING("Cannot destroy TPool object (%x) while working async. Maybe missing a DCS::Threading::JoinPool(TPool*) call.", pool);
}
