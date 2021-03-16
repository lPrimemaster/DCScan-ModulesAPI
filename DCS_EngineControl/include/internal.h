#pragma once
#include "../include/DCS_ModuleEngineControl.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

#include <Windows.h>
#include <queue>

/**
 * @file
 * \internal
 * \brief Internal serial COM TX/RX.
 *
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date 2020/11/26
 * $Modified: 2020/10/19$
 */

namespace DCS
{
	/**
	 * \internal.
	 * \brief %Serial Control.
	 */
	namespace Serial
	{
		/**
		 * \internal
		 * \brief All serial arguments for windows manipulation.
		 */
		struct DCS_INTERNAL_TEST SerialArgs
		{
			DWORD baudRate;
			BYTE byteSize;
			BYTE stopBits;
			BYTE parity;

			CHAR eofChar;

			DWORD readIntervalTimeout;
			DWORD readTotalTimeoutConstant;
			DWORD readTotalTimeoutMultiplier;
			DWORD writeTotalTimeoutConstant;
			DWORD writeTotalTimeoutMultiplier;
		};

		/**
		 * \internal
		 * \brief Initialized COM HANDLE with params.
		 */
		DCS_INTERNAL_TEST HANDLE init_handle(LPCSTR portName, DWORD rwAccess, SerialArgs args);

		/**
		 * \internal
		 * \brief Writes to COM HANDLE with params.
		 */
		DCS_INTERNAL_TEST BOOL   write_bytes(HANDLE hComm, LPCSTR charArray, DWORD NbytesToWrite);

		/**
		 * \internal
		 * \brief Reads from COM HANDLE with params.
		 */
		DCS_INTERNAL_TEST BOOL   read_bytes(HANDLE hComm, LPTSTR buffer, DWORD bufferSize, LPDWORD readBufferSize);

		/**
		 * \internal
		 * \brief Closes COM HANDLE.
		 */
		DCS_INTERNAL_TEST BOOL   close_handle(HANDLE hComm);

		/**
		 * \internal
		 * \brief Enumerates all COM ports by name.
		 */
		DCS_INTERNAL_TEST i32    enumerate_ports(char* buffer, DCS::i32 buff_size);

		DCS_INTERNAL_TEST void   comnumber_to_string(char pname[7], DCS::u8 n);
	}

	/**
	 * \internal
	 * \brief Allows for communications request with COM devices in the local machine.
	 */
	namespace Coms
	{
		struct DCS_INTERNAL_TEST COMDevicePrivateProperties
		{
			u8 comport_esp301;
			u8 comport_pmc8742;
			Serial::SerialArgs serial_args;
		};

		class CmdBuffer;

		DCS_INTERNAL_TEST CmdBuffer& GetCmdBuffer();

		struct DCS_INTERNAL_TEST Command
		{
			Command()
			{
				id = 0;
				full_cmd = "";
				axis = 0;
				target = Control::UnitTarget::ESP301;
			}

			template<typename T>
			Command(Control::UnitTarget target, const char* cmd, u16 axis, T argument)
			{
				this->target = target;
				this->axis = axis;
				this->id = NextId();
				this->wait_response = false;

				full_cmd = std::to_string(axis) + cmd + std::to_string(argument) + ";";
			}

			template<>
			Command(Control::UnitTarget target, const char* cmd, u16 axis, const char* argument)
			{
				this->target = target;
				this->axis = axis;
				this->id = NextId();

				if(argument[0] == '?')
					this->wait_response = true;
				else
					this->wait_response = false;

				full_cmd = std::to_string(axis) + cmd + argument + ";";
			}

			Command(Control::UnitTarget target, const char* cmd, u16 axis = 0)
			{
				this->target = target;
				this->axis = axis;
				this->id = NextId();
				this->wait_response = false;

				full_cmd = (axis > 0 ? std::to_string(axis) : "") + cmd + ";";
			}

			template<typename T>
			Command(Control::UnitTarget target, const char* cmd, T argument)
			{
				this->target = target;
				this->axis = 0;
				this->id = NextId();
				this->wait_response = false;

				full_cmd = cmd + std::to_string(argument) + ";";
			}

			template<>
			Command(Control::UnitTarget target, const char* cmd, const char* argument)
			{
				this->target = target;
				this->axis = 0;
				this->id = NextId();

				if (argument[0] == '?')
					this->wait_response = true;
				else
					this->wait_response = false;

				full_cmd = cmd + std::string(argument) + ";";
			}

			static Command Custom(Control::UnitTarget target, const char* cmd, bool response = false)
			{
				Command c;
				c.target = target;
				c.axis = 0;
				c.id = NextId();
				c.wait_response = response;

				c.full_cmd = cmd;
				return c;
			}

			static const u64 NextId()
			{
				static u64 nid = 0;
				return nid++;
			}

			Command operator+(const Command& rhs)
			{
				if (target != rhs.target)
				{
					LOG_ERROR("Cannot concatenate two commands for different targets.");
					return Command();
				}
				else
				{
					Command res;
					res.id = rhs.id;
					res.axis = 0;
					res.wait_response = wait_response || rhs.wait_response;
					res.full_cmd = full_cmd + rhs.full_cmd;
					return res;
				}
			}


#pragma warning( push )
#pragma warning( disable : 4251 )
			std::string full_cmd;
#pragma warning( pop )

			u64 id;
			u16 axis;
			Control::UnitTarget target;
			bool wait_response;
		};

#pragma warning( push )
#pragma warning( disable : 4251 )
		class DCS_INTERNAL_TEST CmdBuffer
		{
		public:
			using Type = Command;
			CmdBuffer() {};
			~CmdBuffer() {};

			Type process()
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

			std::string schedule(const Type& value)
			{
				std::unique_lock<std::mutex> lock(m);
				q.push(value);
				lock.unlock();
				c.notify_one();

				if (value.wait_response)
				{
					LOG_DEBUG("Waiting for response...");
					std::unique_lock<std::mutex> lock(mr);
					cr.wait(lock);
					LOG_DEBUG("Finished waiting...");
					return response_buffer;
				}

				response_buffer = "";
				return response_buffer;
			}

			void reply(std::string response)
			{
				std::unique_lock<std::mutex> lock(mr);
				response_buffer = response;
				lock.unlock();
				cr.notify_one();
				LOG_DEBUG("Notifying...");
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

			std::mutex mr;
			std::condition_variable cr;
			std::string response_buffer;
		};
#pragma warning( pop )
	}
}