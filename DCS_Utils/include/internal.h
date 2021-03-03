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
		class DCS_INTERNAL_TEST SMessageQueue
		{
		public:
			using Type = DCS::Network::Message::DefaultMessage;
			SMessageQueue() {};
			~SMessageQueue() {};

			Type pop()
			{
				std::unique_lock<std::mutex> lock(m);
				while (q.empty())
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

			void push(const Type& value)
			{
				std::unique_lock<std::mutex> lock(m);
				q.push(value);
				lock.unlock();
				c.notify_one();
			}

			void notify_unblock()
			{
				c.notify_one();
			}

			int size()
			{
				std::unique_lock<std::mutex> lock(m);
				int s = q.size();
				lock.unlock();
				return s;
			}

		private:
			std::queue<Type> q;
			std::mutex m;
			std::condition_variable c;
		};


		template<typename E>
		constexpr auto toUnderlyingType(E e)
		{
			return static_cast<typename std::underlying_type<E>::type>(e);
		}
	}
}
