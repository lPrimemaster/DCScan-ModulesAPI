#ifndef _DCS_CORE_H
#define _DCS_CORE_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

/**
 * @file
 * \brief Exposes core functionalities of the API to the end user.
 *
 * \author Cesar Godinho
 *
 * \version 1.0 $Revision: 1.1 $
 *
 * \date $Date: 2020/11/26$
 */

namespace DCS
{
	namespace Memory
	{
		/**
		 * \internal
		 * \brief Struct to store data linearly
		 *
		 * Works as memory storage for the LinearAlocator.
		 */
		struct LinearDataPointer
		{
			char* data_start = nullptr;
			char* data = nullptr;
			u64 size;
			u64 usedSize;
			u64 alignment;
		};

		/**
		 * \brief Linearly allocates memory for a single use.
		 * 
		 * Can be reused upon reset. Can't be realoc'ed.
		 */
		class DCS_API LinearAllocator
		{
		public:
			/**
			 * \brief Creates a new LinearAllocator.
			 * \param size Size of the linear pool to alocate
			 * \param align Align the data to align-bits. (Currently disabled.)
			 */
			LinearAllocator(u64 size, u64 align);

			/**
			 * \brief Allocates a new section of the pool for a new type T.
			 * \tparam T Type to allocate.
			 * \param args Arguments to the costructor of T.
			 */
			template<typename T, typename... Args>
			T* allocate(Args... args)
			{
				buffer.usedSize += sizeof(T);

				if (buffer.usedSize > buffer.size)
				{
					buffer.usedSize -= sizeof(T);
					LOG_CRITICAL("Allocating %u bytes would result in buffer overrun. [Available: %u]", sizeof(T), buffer.size - buffer.usedSize);
					return nullptr;
				}

				T* rb = nullptr;
				rb = new(buffer.data) T(args...);

				if (rb == nullptr)
				{
					LOG_CRITICAL("Failed to allocated %u bytes in allocator.", sizeof(T));
					return nullptr;
				}

				buffer.data += sizeof(T);
				return rb;
			}

			/**
			 * \brief Allocates a new section of the pool for a new type T.
			 * \tparam T Type to allocate.
			 */
			template<typename T>
			T* allocate()
			{
				buffer.usedSize += sizeof(T);

				if (buffer.usedSize > buffer.size)
				{
					buffer.usedSize -= sizeof(T);
					LOG_CRITICAL("Allocating %u bytes would result in buffer overrun. [Available: %u]", sizeof(T), buffer.size - buffer.usedSize);
					return nullptr;
				}

				T* rb = nullptr;
				rb = new(buffer.data) T();

				if (rb == nullptr)
				{
					LOG_CRITICAL("Failed to allocated %u bytes in allocator.", sizeof(T));
					return nullptr;
				}

				buffer.data += sizeof(T);
				return rb;
			}

			/**
			 * \brief Free all the linear allocated memory of the buffer.
			 * 
			 * This invalidates the buffer data.
			 */
			void release();

			/**
			 * \brief Invalidate all pool data. Return buffer pointer to start.
			 * 
			 * Effectively setting memory usage to zero.
			 */
			void reset();

		private:
			LinearDataPointer buffer;
		};
	}

	namespace Threading
	{
		/**
		 * \brief Get current machine maximum hardware concurrency (Number of physical threads supported by the current implementation).
		 */
		DCS_REGISTER_CALL(DCS::u16)
		const DCS_API u16 GetMaxHardwareConcurrency();


		// TODO : Remove these testing functions
		DCS_REGISTER_CALL(int, int, int)
		const DCS_API int addInt(int a, int b);

		DCS_REGISTER_CALL(void, float)
		DCS_API void displayFloat(float v);
	}
}

#endif _DCS_CORE_H