#pragma once
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <functional>
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

namespace DCS
{
	namespace Memory
	{
		/**
		 * \brief Holds data pointer and data pointer information for linear memory pools.
		 */
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

		DCS_INTERNAL_TEST TPool* CreatePersistentThread(u16 size, std::vector<std::function<void(std::mutex*, std::condition_variable*, std::array<std::atomic_int, 16>*)>> workers);

		DCS_INTERNAL_TEST void JoinPool(TPool* pool);
	}
}