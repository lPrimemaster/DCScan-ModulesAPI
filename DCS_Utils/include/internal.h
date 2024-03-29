#pragma once
#include "DCS_ModuleUtils.h"
#include "../../DCS_Network/include/internal.h"

#include <queue>
#include <mutex>
#include <condition_variable>

/**
 * @file
 * \internal
 * \brief Exposes internal functionalities of the Utils namespace.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2020/11/01$
 */

/**
 * \internal
 * \brief Creates a opaque generic handle only usefull to the API.
 * 
 * This is usefull for passing STL pointers to the client to hold API states.
 */
DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj = nullptr);

/**
 * \internal
 * \brief Frees a generic handle's memory.
 */
void FreeGenericHandle(DCS::GenericHandle hnd);

namespace DCS
{
	namespace Utils
	{
#pragma warning( push )
#pragma warning( disable : 4251 )

		/**
		 * \internal 
		 * \brief A thread-safe message queue. Similar to std::queue.
		 */
		template<typename Type>
		class DCS_INTERNAL_TEST SMessageQueue
		{
		public:
			// using Type = DCS::Network::Message::DefaultMessage;
			SMessageQueue() {};
			~SMessageQueue() {};

			Type pop()
			{
				std::unique_lock<std::mutex> lock(m);
				if (q.empty())
				{
					c.wait(lock);
				}
				if (!q.empty())
				{
					Type copy = q.front();
					q.pop();
					return copy;
				}
				return Type();
			}

			Type peek()
			{
				std::unique_lock<std::mutex> lock(m);
				if (q.empty())
				{
					c.wait(lock);
				}
				if (!q.empty())
				{
					Type copy = q.front();
					return copy;
				}
				return Type();
			}

			Type peekBack()
			{
				std::unique_lock<std::mutex> lock(m);
				if (q.empty())
				{
					c.wait(lock);
				}
				if (!q.empty())
				{
					Type copy = q.back();
					return copy;
				}
				return Type();
			}

			void push(const Type& value)
			{
				std::unique_lock<std::mutex> lock(m);
				q.push(value);
				lock.unlock();
				c.notify_one();
			}

			void notify_unblock()
			{
				c.notify_all();
			}

			int size()
			{
				std::unique_lock<std::mutex> lock(m);
				int s = (int)q.size();
				lock.unlock();
				return s;
			}

		private:
			std::queue<Type> q;
			std::mutex m;
			std::condition_variable c;
		};

		/**
		 * \internal 
		 * \brief A thread-safe byte queue. Holds message data up to a certain specified header size.
		 */
		class DCS_INTERNAL_TEST ByteQueue
		{
		public:
			ByteQueue(u64 buffer_size)
			{ 
				buffer = new u8[buffer_size]; 
				internal_buff_size = 0;
				internal_buff_max_size = buffer_size; 
				ntf_unblock.store(false);
			};
			~ByteQueue() 
			{ 
				delete[] buffer;
				internal_buff_size = 0;
				internal_buff_max_size = 0;
			};

			void addBytes(u8* data, u64 size)
			{
				std::unique_lock<std::mutex> lock(m);

				c.wait(lock, [&] {return internal_buff_size + size <= internal_buff_max_size; });

				memcpy(buffer + internal_buff_size, data, size);
				internal_buff_size += size;

				lock.unlock();
				c.notify_one();
			}

			u64 fetchNextMsg(u8* buffer_dst)
			{
				std::unique_lock<std::mutex> lock(m);

				c.wait(lock, [&] {return (internal_buff_size >= sizeof(i32) || ntf_unblock.load()); });

				if (ntf_unblock.load())
				{
					return 0;
				}

				i32 to_read;
				memcpy(&to_read, buffer, sizeof(i32));
				const i32 packet_size = to_read + sizeof(i32);

				c.wait(lock, [&] { return packet_size <= internal_buff_size; });

				memcpy(buffer_dst, buffer + sizeof(i32), to_read);
				internal_buff_size -= packet_size;

				if (internal_buff_size > 0)
					memmove(buffer, buffer + packet_size, internal_buff_size);

				lock.unlock();
				c.notify_one();

				return to_read;
			}

			u64 count()
			{
				std::unique_lock<std::mutex> lock(m);
				u64 ibs = internal_buff_size;
				lock.unlock();
				return ibs;
			}

			void notify_unblock()
			{
				ntf_unblock.store(true);
				c.notify_all();
			}

			void notify_restart()
			{
				delete[] buffer;
				internal_buff_size = 0;
				buffer = new u8[internal_buff_max_size]; 
				ntf_unblock.store(false);
			}

		private:
			u8* buffer;
			u64 internal_buff_size;
			u64 internal_buff_max_size;

			std::atomic<bool> ntf_unblock;
			std::mutex m;
			std::condition_variable c;
		};
#pragma warning( pop )

		template<typename E>
		constexpr auto toUnderlyingType(E e)
		{
			return static_cast<typename std::underlying_type<E>::type>(e);
		}

		DCS_INTERNAL_TEST void GetConsoleSize(int* x, int* y);

		DCS_INTERNAL_TEST void SetConsoleMargins(int t, int b);

		DCS_INTERNAL_TEST void WriteConsoleLine(int b, const char* str);

		DCS_INTERNAL_TEST void SetStdinEcho(bool enable);
	}

	namespace Timer
	{
		/**
		 * \brief Holds timing data.
		 *
		 * Used to get relative timestamps with 100 nanoseconds precision.
		 */
		class DCS_INTERNAL_TEST SystemTimer
		{
		public:
			SystemTimer() = default;
			~SystemTimer() = default;

			/**
			 * \brief Starts/restarts timer. Setting timing zero as now.
			 */
			void start();

			/**
			 * \brief Gives a timestamp relative to timer in Timestamp format.
			 * 
			 * \return Timestamp.
			 */
			Timestamp getTimestamp();

			/**
			 * \brief Gives a timestamp relative to timer in Utils::String format.
			 *
			 * \return Utils::String timestamp [XXh XXm XXs XXms XXus XXns].
			 */
			Utils::String getTimestampString();

			
			/**
			 * \brief Gives a timestamp relative to timer in Utils::String format (displays day/hour/minute duration only).
			 * 
			 * \return Utils::String timestamp [XXd XXh XXm].
			 */
			Utils::String getTimestampStringSimple();

			/**
			 * \brief Gives number of nanoseconds passed relative to timer.
			 *
			 * \return Number of nanoseconds stored in a DCS::i64.
			 */
			i64 getNanoseconds();

		private:
			std::chrono::time_point<std::chrono::steady_clock> point;
		};
	}
}
