#pragma once
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "DCS_ModuleNetwork.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <atomic>
#include <queue>

/**
 * @file
 * \internal
 * \brief Exposes internal functionalities of the Network namespace.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2021/02/22$
 */

namespace DCS
{
	namespace Network
	{
		/**
		 * \internal.
		 * \brief Holds windows WSA data.
		 */
		struct DCS_INTERNAL_TEST WindowsSocketInformation
		{
			WSADATA wsa;
		};

		inline static bool is_inited = false;
		inline static std::atomic<i32> instances = 0;

		/**
		 * \internal.
		 * \brief Initializes windows WinSock2.
		 */
		DCS_INTERNAL_TEST WSADATA InitWinSock();

		/**
		 * \internal.
		 * \brief Cleans up windows WinSock2.
		 */
		DCS_INTERNAL_TEST void CleanupWinSock();


		/**
		 * \internal.
		 * \brief Creates a server socket.
		 */
		DCS_INTERNAL_TEST SOCKET CreateServerSocket(i32 listen_port);

		/**
		 * \internal.
		 * \brief Creates a client socket.
		 */
		DCS_INTERNAL_TEST SOCKET CreateClientSocket(const char* host, i32 port);

		/**
		 * \internal.
		 * \brief Allow server socket to start listenning.
		 */
		DCS_INTERNAL_TEST void ServerListen(SOCKET server);

		/**
		 * \internal.
		 * \brief Allow server socket to accept inbound connections.
		 */
		DCS_INTERNAL_TEST SOCKET ServerAcceptConnection(SOCKET server);

		/**
		 * \internal.
		 * \brief Closes a socket connection.
		 */
		DCS_INTERNAL_TEST void CloseSocketConnection(SOCKET client);

		/**
		 * \internal.
		 * \brief Checks a socket status.
		 */
		DCS_INTERNAL_TEST bool ValidateSocket(SOCKET s);

		/**
		 * \internal.
		 * \brief Allow server to receive data from a client socket.
		 */
		DCS_INTERNAL_TEST i32 ReceiveData(SOCKET client, unsigned char* buffer, i32 buff_len);

		/**
		 * \internal.
		 * \brief Allow server to send data trought a client socket.
		 */
		DCS_INTERNAL_TEST i32 SendData(SOCKET client, const unsigned char* buffer, i32 buff_len);

		/**
		 * \internal.
		 * \brief Allow server to send data trought a client socket using OOB data.
		 * 
		 * This can be used when a message sent to the server is of high priority, 
		 * such as a remote emergency shutdown.
		 */
		DCS_INTERNAL_TEST i32 SendPriorityData(SOCKET client, const unsigned char* buffer, i32 buff_len);

		namespace Message
		{
#pragma pack(push, 1)
			// TODO : Consider using this as a Message for both client and server (part of the API)
			struct DCS_INTERNAL_TEST DefaultMessage
			{
				u8 op;
				i64 size;
				u8* ptr;
			};
#pragma pack(pop)

			DCS_INTERNAL_TEST DefaultMessage Alloc(i32 size);
			DCS_INTERNAL_TEST void Set(DefaultMessage& msg, u8* data);
			DCS_INTERNAL_TEST void Set(DefaultMessage& msg, u8 opcode, u8* data);
			DCS_INTERNAL_TEST void Delete(DefaultMessage msg);

			DCS_INTERNAL_TEST void ScheduleTransmission(DefaultMessage msg);
		}
	}
}
