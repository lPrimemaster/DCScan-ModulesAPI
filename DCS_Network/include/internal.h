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
using Logger = DCS::Utils::Logger;

namespace DCS
{
	namespace Network
	{
		struct DCS_INTERNAL_TEST WindowsSocketInformation
		{
			WSADATA wsa;
		};

		DCS_INTERNAL_TEST WindowsSocketInformation InitWinSock();

		DCS_INTERNAL_TEST void CleanupWinSock(SOCKET* open_sockets, u16 num_sockets);

		DCS_INTERNAL_TEST SOCKET CreateServerSocket(i32 listen_port);

		DCS_INTERNAL_TEST void ServerListen(SOCKET server);

		DCS_INTERNAL_TEST SOCKET ServerAcceptConnection(SOCKET server);

		DCS_INTERNAL_TEST i64 ServerReceiveData(SOCKET client, char* buffer, i16 buff_len);

		//DCS_INTERNAL_TEST void ServerSendData(SOCKET client, char* buffer, u64 buff_len);

		//DCS_INTERNAL_TEST void CreateClientSocket(i32 port);
	}
}
