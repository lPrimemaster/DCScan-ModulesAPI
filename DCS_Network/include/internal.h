#pragma once
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "DCS_ModuleNetwork.h"
#include "../config/registry.h"

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

/**
 * \internal
 */
#define _GET_F_NAME() __FUNCTION__

/**
  * \internal
  */
#define GET_F_NAME() _GET_F_NAME()

/**
 * \internal
 * \brief Placed anywhere to emit the registered event 'name'.
 * \see DCS_REGISTER_EVENT
 */
#define DCS_EMIT_NAMED_EVT(name, data, size) DCS::Network::Message::EmitEvent(name, data, size)

/**
  * \internal
  * \brief Placed in the event main caller to emit the registered associated event.
  * \see DCS_REGISTER_EVENT
  */
#define DCS_EMIT_EVT(data, size) DCS::Network::Message::EmitEvent(DCS::Registry::GetEvent(GET_F_NAME()), data, size)

extern "C" 
{ 
	typedef struct WSAData WSADATA;
	typedef DCS::u64 SOCKET;
}

namespace DCS
{
	namespace Network
	{
		inline static bool is_inited = false;

		/**
		 * \internal
		 * \brief Initializes windows WinSock2.
		 */
		DCS_INTERNAL_TEST WSADATA InitWinSock();

		/**
		 * \internal
		 * \brief Cleans up windows WinSock2.
		 */
		DCS_INTERNAL_TEST void CleanupWinSock();

		/**
		 * \internal
		 * \brief Creates a server socket.
		 */
		DCS_INTERNAL_TEST SOCKET CreateServerSocket(i32 listen_port);

		/**
		 * \internal
		 * \brief Creates a client socket.
		 */
		DCS_INTERNAL_TEST SOCKET CreateClientSocket(const char *host, i32 port);

		/**
		 * \internal
		 * \brief Allow server socket to start listenning.
		 */
		DCS_INTERNAL_TEST void ServerListen(SOCKET server);

		/**
		 * \internal
		 * \brief Allow server socket to accept inbound connections.
		 */
		DCS_INTERNAL_TEST SOCKET ServerAcceptConnection(SOCKET server);

		/**
		 * \internal
		 * \brief Closes a socket connection.
		 */
		DCS_INTERNAL_TEST void CloseSocketConnection(SOCKET client);

		/**
		 * \internal
		 * \brief Checks a socket status.
		 */
		DCS_INTERNAL_TEST bool ValidateSocket(SOCKET s);

		/**
		 * \internal
		 * \brief Allow server to receive data from a client socket.
		 */
		DCS_INTERNAL_TEST i32 ReceiveData(SOCKET client, unsigned char *buffer, i32 buff_len);

		/**
		 * \internal
		 * \brief Allow server to send data trought a client socket.
		 */
		DCS_INTERNAL_TEST i32 SendData(SOCKET client, const unsigned char *buffer, i32 buff_len);

		/**
		 * \internal
		 * \brief Allow server to send data trought a client socket using OOB data.
		 * 
		 * This can be used when a message sent to the server is of high priority, 
		 * such as a remote emergency shutdown.
		 */
		DCS_INTERNAL_TEST i32 SendPriorityData(SOCKET client, const unsigned char *buffer, i32 buff_len);

		/**
		 * \internal
		 * \brief Get the Socket Ip Address
		 * 
		 * \param s socket
		 * \param buffer buffer to write the ip string to
		 */
		DCS_INTERNAL_TEST void GetSocketIpAddress(SOCKET s, char* buffer);

		namespace Message
		{

#define MESSAGE_XTRA_SPACE 9
#pragma pack(push, 1)
			/**
			 * \internal
			 * \brief A default nominal message encoded to send a varible size of data.
			 */
			struct DCS_INTERNAL_TEST DefaultMessage
			{
				u8 op;
				u64 id; // used for sync call message identify
				i64 size;
				u8 *ptr;
			};
#pragma pack(pop)

			extern std::mutex message_m;
			extern std::condition_variable lsync;
			extern DefaultMessage lmessage;

			/**
			 * \internal
			 * \brief A default operation for internal use only.
			 */
			enum class DCS_INTERNAL_TEST InternalOperation
			{
				NO_OP = 0,			///< Ping the server only.
				SYNC_REQUEST = 2,	///< Request a synchronous function call to the server, waiting for the result.
				ASYNC_REQUEST = 3,	///< Request an asynchronous function call to the server.
				SYNC_RESPONSE = 5,	///< Send back a sync response to the client.
				ASYNC_RESPONSE = 6, ///< Send back an async response to the client.
				EVT_SUB = 7,		///< Subscribe to a server-side event.
				EVT_RESPONSE = 8,	///< Send event response to the client.
				EVT_UNSUB = 9,		///< Unsubscribe from a previously subscribed event.
				OP_ERROR,			///< Send an error to the client/server.
				CON_VALID,			///< Server connection validity message.
				DATA				///< Send or receive data only.
			};

			/**
			 * \internal
			 * \brief Wait for a return message with id.
			 * \todo This will no longer work if the SVReturn uses pointers instead of full data
			 */
			DCS_INTERNAL_TEST Registry::SVReturn WaitForId(u64 id);

			/**
			 * \internal
			 * \brief Set the last arrived DefaultMessage as msg, and notify all of WaitForId(u64) calls.
			 */
			DCS_INTERNAL_TEST void SetMsgIdCondition(DefaultMessage &msg);

			/**
			 * \internal
			 * \brief Similar to SetMsgIdCondition, but works for async requests.
			 */
			DCS_INTERNAL_TEST void NotifyPromise(DefaultMessage &msg);

			/**
			 * \internal
			 * \brief Alocates space for a message. This size must include the sizeof(opcode) and sizeof(id).
			 * These added sizes are also available via the MESSAGE_XTRA_SPACE definition.
			 */
			DCS_INTERNAL_TEST DefaultMessage Alloc(i32 size);

			/**
			 * \internal
			 * \brief Copy data to msg (where data contains the id and opcode).
			 */
			DCS_INTERNAL_TEST void SetCopyIdAndCode(DefaultMessage &msg, u8 *data);

			/**
			 * \internal
			 * \brief Copy data to msg (keeping the id passed).
			 * Copies only the data pointer.
			 */
			DCS_INTERNAL_TEST void SetCopyId(DefaultMessage &msg, u8 opcode, u64 id, u8 *data);

			/**
			 * \internal
			 * \brief Create new msg with data and an opcode (incrementing the id).
			 */
			DCS_INTERNAL_TEST void SetNew(DefaultMessage &msg, u8 opcode, u8 *data);

			/**
			 * \internal
			 * \brief Deep copy msg.
			 */
			DCS_INTERNAL_TEST DefaultMessage Copy(DefaultMessage &msg);

			/**
			 * \internal
			 * \brief Delete msg.
			 */
			DCS_INTERNAL_TEST void Delete(DefaultMessage &msg);

			/**
			 * \internal
			 * \brief Schedules the transmission of a message to the client send thread.
			 */
			DCS_INTERNAL_TEST void ScheduleTransmission(DefaultMessage msg);

			/**
			 * \internal
			 * \brief Schedules the emission of an event to the client thread.
			 */
			DCS_INTERNAL_TEST void EmitEvent(u8 EVT_ID, u8 *evtData, i32 size);
		}
	}
}
