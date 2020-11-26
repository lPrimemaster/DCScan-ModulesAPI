#pragma once
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <functional>
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

/**
 * @file
 * 
 * \internal
 * 
 * \brief Internal file eposing multiple functionalities (see specific documentation of members in this file \todo Doc members here.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2020/11/15$
 */
namespace DCS
{
	namespace Memory
	{
		struct LinearDataPointer
		{
			char* data_start = nullptr;
			char* data = nullptr;
			u64 size;
			u64 alignment;
		};
	}

	namespace Threading
	{
		struct TSignal
		{

		};

		enum NotifyFlag
		{
			START
		};

		struct DCS_INTERNAL_TEST TPool
		{
			std::vector<std::thread> workers;
			std::mutex lock;
			std::condition_variable signal;
			std::array<std::atomic_int, 16> flags;
		};

		DCS_INTERNAL_TEST TPool* CreatePersistentPool(u16 size, std::vector<std::function<void(std::mutex*, std::condition_variable*, std::array<std::atomic_int, 16>*)>> workers);

		DCS_INTERNAL_TEST void JoinPool(TPool* pool);

		DCS_INTERNAL_TEST u64 GetPoolWorkCount(TPool* pool);

		DCS_INTERNAL_TEST void DestroyPool(TPool* pool);
	}
}