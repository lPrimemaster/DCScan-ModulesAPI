#pragma once
#include <thread>
#include "../include/DCS_ModuleCore.h"

const DCS::i16 DCS::Threading::GetMaxHardwareConcurrency()
{
	return std::thread::hardware_concurrency();
}

DCS::Threading::TPool* DCS::Threading::CreatePersistentThread(u16 size, std::vector<std::function<void(std::mutex*, std::condition_variable*, std::array<std::atomic_int, 16>*)>> workers)
{
	TPool* pool = new TPool();

	for(auto w : workers)
		pool->workers.push_back(std::thread(w, &pool->lock, &pool->signal, &pool->flags));

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

void DCS::Threading::DestroyPool(TPool* pool)
{
	if (!pool->workers.size())
		delete pool;
	else
		DCS::Utils::Logger::Warning("Cannot destroy TPool object while working async. Maybe missing a DCS::Threading::JoinPool(TPool*) call.");
}