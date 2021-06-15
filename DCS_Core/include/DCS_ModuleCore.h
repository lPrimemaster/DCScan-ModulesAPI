#ifndef _DCS_CORE_H
#define _DCS_CORE_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

#include <functional>

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
		// class CircularBuffer
		// {
		// public:
		// 	CircularBuffer(u64 block_size, u64 num_blocks);
		// 	~CircularBuffer();

		// 	inline u8* write(u8* data, u64 sz)
		// 	{
		// 		if(sz > block_size)
		// 		{
		// 			LOG_CRITICAL("CircularBuffer: Write operation size exceeded block_size.");
		// 			return nullptr;
		// 		}

		// 		while(read_tag.load() & (1 << o_write))
		// 		{
		// 			std::this_thread::yield();
		// 		}

		// 		std::unique_lock<std::mutex> lck(m);

		// 		cv.wait(lck, []() { return (read_tag.load() & (1 << o_write)) == 0; });

		// 		lck.unlock();
		// 		u8* p = data + (o_write++ * block_size);
		// 		memcpy(p, data, sz);

		// 		read_tag |= (1 << (o_write-1));

		// 		o_write %= num_blocks;
		// 		write_cycle++;
		// 		return p;
		// 	}

		// 	inline void read(u8* dst_buffer, u64 sz)
		// 	{
		// 		if(dst_buffer == nullptr)
		// 		{
		// 			LOG_CRITICAL("CircularBuffer: Read operation destination buffer is NULL.");
		// 			return;
		// 		}

		// 		std::unique_lock<std::mutex> lck(m);

		// 		cv.wait(lck, []() { return (read_tag.load() & (1 << o_read)) == 1; });


		// 		u8* p = data + (o_read++ * block_size);
		// 		memcpy(dst_buffer, p, sz);
		// 		read_tag &= ~(1 << (o_read-1));

		// 		o_read %= num_blocks;
		// 		read_cycle++;
				
		// 		lck.unlock();
		// 	}

		// private:
		// 	u8* data = nullptr;
		// 	u64 block_size = 0;
		// 	u64 num_blocks = 0;
		// 	u64 total_size = 0;

		// 	u64 o_read = 0;
		// 	u64 o_write = 0;

		// 	std::atomic<u64> read_tag = 0xFFFFFFFFFFFFFFFF;

		// 	std::condition_variable cv;
		// 	std::mutex m;

		// 	u64 read_cycle = 0;
		// 	u64 write_cycle = 0;
		// };


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

	/**
	 * \brief Handles threading and asynchronicity.
	 */
	namespace Threading
	{
		/**
		 * \brief Get current machine maximum hardware concurrency (Number of physical threads supported by the current implementation).
		 * \ingroup calls
		 */
		DCS_REGISTER_CALL(DCS::u16)
		const DCS_API u16 GetMaxHardwareConcurrency();
	}

	/**
	 * \brief Handles authentication and cryptography.
	 */
	namespace Auth
	{
		/**
		 * \brief Encrypts bytes with the AES-256 (GCM) cypher using a key and iv.
		 * \deprecated This function is no longer required to be called directly on the client.
		 */
		DCS_API void Encrypt(DCS::u8* to_encrypt, int size, DCS::u8* key, DCS::u8* iv, DCS::u8* encrypted_out, DCS::u8* tag);
	}

	/**
	 * \brief Holds math utilities and tools.
	 */
	namespace Math
	{
		/**
		 * \brief Struct that holds the data retrieved from DCS::Math::countArrayPeak.
		 * 
		 * Automatically cleans up used memory.
		 */
		struct CountResult
		{
			u64 num_detected = 0;	   ///< Number of detected peaks.
			u64* maximizers = nullptr; ///< x positions array of the detected peaks (in samples).
			f64* maxima = nullptr;     ///< y values array of the detected peaks.

			~CountResult()
			{
				if(maximizers != nullptr)
				{
					delete[] maximizers;
				}

				if(maxima != nullptr)
				{
					delete[] maxima;
				}
			}
		};
		
		/**
		 * \brief Counts the total ocurrences of peaks between vlo and vhi, with the specified sensitivity threshold.
		 * 
		 * Has two modes:
		 * Legacy - Slower but more reliable (use by defining '#define DCS_MATH_USE_LEGACY_COUNTER' in the server).
		 * Core   - Faster but can fail n certain edge scenarios (default).
		 * 
		 * Remark: Only the legacy mode is currently supported (and is enabled by default).
		 * 
		 * \param arr The data array to be analyzed.
		 * \param size The size of the data array.
		 * \param vlo The low limit to consider peaks.
		 * \param vhi The high lmti to consider peaks.
		 * \param vth The sensitivity threshold.
		 * 
		 * \return A DCS::Math::CountResult struct with data of analyzed array.
		 */
		CountResult countArrayPeak(f64* arr, u64 size, f64 vlo, f64 vhi, f64 vth);
	}
}

#endif _DCS_CORE_H