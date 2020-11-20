#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/internal.h"

namespace DCS
{
	namespace Memory
	{
		/**
		 * \brief Linearly allocates memory for a single use.
		 * 
		 * Can be resused upon reset. Can't be realoc'ed.
		 */
		struct DCS_API LinearAllocator
		{
		public:
			/**
			 * \brief Creates a new LinearAllocator.
			 * \param size Size of the linear pool to alocate
			 * \param align Align the data to align-bits. (Currently disabled.)
			 */
			static LinearAllocator New(u64 size, u64 align = 1);

			/**
			 * \brief Allocates a new section of the pool for a new type T.
			 * \tparam T Type to allocate.
			 * \param args Arguments to the costructor of T.
			 */
			template<typename T, typename... Args>
			T* allocate(Args... args)
			{
				T* rb = new(buffer.data) T(args...);
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
				T* rb = new(buffer.data) T();
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
			LinearAllocator(u64 size, u64 align);

		private:
			LinearDataPointer buffer;
		};
	}

	namespace Threading
	{
		const i16 GetMaxHardwareConcurrency();
	}
}