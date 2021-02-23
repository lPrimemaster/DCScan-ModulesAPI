#pragma once
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "DCS_ModuleNetwork.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

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

		/**
		 * \internal.
		 * \brief Initializes windows WinSock2.
		 */
		DCS_INTERNAL_TEST WindowsSocketInformation InitWinSock();

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
		DCS_INTERNAL_TEST i64 ServerReceiveData(SOCKET client, char* buffer, i16 buff_len);

		/**
		 * \internal.
		 * \brief Allow server to send data trought a client socket.
		 */
		DCS_INTERNAL_TEST i64 ServerSendData(SOCKET client, char* buffer, i16 buff_len);

		/**
		 * \internal.
		 * \brief Allow server to send data trought a client socket using OOB data.
		 * 
		 * This can be used when a message sent to the server is of high priority, 
		 * such as a remote emergency shutdown.
		 */
		DCS_INTERNAL_TEST i64 ServerSendPriorityData(SOCKET client, char* buffer, i16 buff_len);
	}
}
