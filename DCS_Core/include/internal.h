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
	namespace Threading
	{
		struct TSignal
		{

		};

		enum NotifyFlag
		{
			START
		};

		/**
		 * \internal
		 * \brief A struct that holds data for a worker thread pool.
		 */
		struct DCS_INTERNAL_TEST TPool
		{
			std::vector<std::thread> workers;
			std::mutex lock;
			std::condition_variable signal;
			std::array<std::atomic_int, 16> flags;
		};

		/**
		 * \internal
		 * \brief Creates a fixed size thread pool.
		 * 
		 * All the passed functions are immutable in this mode. Making it perfect for long term 
		 * jobs with repeating tasks.
		 */
		DCS_INTERNAL_TEST TPool* CreatePersistentPool(u16 size, std::vector<std::function<void(std::mutex*, std::condition_variable*, std::array<std::atomic_int, 16>*)>> workers);

		/**
		 * \internal
		 * \brief Joins all threads inside a thread pool.
		 * 
		 * Refer to std::thread::join().
		 */
		DCS_INTERNAL_TEST void JoinPool(TPool* pool);

		/**
		 * \internal
		 * \brief Get the worker size of a TPool.
		 */
		DCS_INTERNAL_TEST u64 GetPoolWorkCount(TPool* pool);

		/**
		 * \internal
		 * \brief Deletes all data and workers associated with
		 * a certain TPool object.
		 */
		DCS_INTERNAL_TEST void DestroyPool(TPool* pool);
	}
}