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
			// Internal messages configuration
#pragma pack(push, 1)

#define MESSAGE_XTRA_SPACE 9
			struct DCS_INTERNAL_TEST DefaultMessage
			{
				u8 op;
				u64 id; // used for sync call message identify
				i64 size;
				u8* ptr;
			};
#pragma pack(pop)

			extern std::mutex message_m;
			extern std::condition_variable lsync;
			extern DefaultMessage lmessage;

			enum class DCS_INTERNAL_TEST InternalOperation
			{
				NO_OP = 0,			///< Ping the server only.
				SYNC_REQUEST = 2,   ///< Request a synchronous function call to the server, waiting for the result.
				ASYNC_REQUEST = 3,  ///< Request an asynchronous function call to the server.
				SYNC_RESPONSE = 5,	///< Send back a sync response to the client.
				ASYNC_RESPONSE = 6,	///< Send back an async response to the client.
				SUB_EVT,			///< Subscribe to a server-side event.
				UNSUB_EVT,			///< Unsubscribe from a previously subscribed event.
				DATA				///< Send or receive data only.
			};

			// TODO : This will no longer work if the SVReturn uses pointers instead of full data
			DCS_INTERNAL_TEST Registry::SVReturn WaitForId(u64 id);
			DCS_INTERNAL_TEST void SetMsgIdCondition(DefaultMessage& msg);

			DCS_INTERNAL_TEST DefaultMessage Alloc(i32 size);

			DCS_INTERNAL_TEST void SetCopyIdAndCode(DefaultMessage& msg, u8* data);
			DCS_INTERNAL_TEST void SetCopyId(DefaultMessage& msg, u8 opcode, u64 id, u8* data);
			DCS_INTERNAL_TEST void SetNew(DefaultMessage& msg, u8 opcode, u8* data);

			DCS_INTERNAL_TEST DefaultMessage Copy(DefaultMessage& msg);
			DCS_INTERNAL_TEST void Delete(DefaultMessage& msg);

			DCS_INTERNAL_TEST void ScheduleTransmission(DefaultMessage msg);
		}
	}
}
