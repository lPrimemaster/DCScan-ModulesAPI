#include "../include/internal.h"
#include "../include/DCS_ModuleNetwork.h"

void DCS::Network::Init()
{
	if (!is_inited)
	{
		is_inited = true;
		InitWinSock();
	}
	// Silently ignore
}

bool DCS::Network::GetStatus()
{
	return is_inited;
}

void DCS::Network::Destroy()
{
	if (is_inited)
	{
		is_inited = false;
		CleanupWinSock();
	}
	// Silently ignore
}


WSADATA DCS::Network::InitWinSock()
{
	WSADATA wsa;
	
	int iResult = WSAStartup(MAKEWORD(2,2), &wsa);

	if (iResult != 0)
	{
		LOG_ERROR("Windows socket implementation (WSA) failed: %d", iResult);
	}
	else
	{
		LOG_DEBUG("Windows socket implementation (WSA) initialized.");
	}
	return wsa;
}

void DCS::Network::CleanupWinSock()
{
	WSACleanup();
}

SOCKET DCS::Network::CreateServerSocket(i32 listen_port)
{
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve local adress of the server
	int iResult = getaddrinfo(NULL, DCS::Utils::String::From(listen_port).c_str(), &hints, &result);

	if (iResult != 0)
	{
		LOG_ERROR("getaddrinfo (WSAAPI) failed: %d", iResult);
		LOG_ERROR("Terminating WSA...");
		WSACleanup();
		return INVALID_SOCKET;
	}
	LOG_DEBUG("IP Address found.");

	SOCKET ListenSocket = INVALID_SOCKET;

	// Create the socket
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET)
	{
		LOG_ERROR("socket creation (WSAAPI) failed: %ld", WSAGetLastError());
		LOG_ERROR("Terminating WSA...");
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}
	LOG_DEBUG("IPv4 socket created.");

	// Bind the socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	if (iResult == SOCKET_ERROR)
	{
		LOG_ERROR("socket bind (WSAAPI) failed: %d", WSAGetLastError());
		LOG_ERROR("Closing socket...");
		LOG_ERROR("Terminating WSA...");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}
	LOG_DEBUG("Socket bind successful.");
	freeaddrinfo(result);

	return ListenSocket;
}

SOCKET DCS::Network::CreateClientSocket(const char* host, i32 port)
{
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve adress of the server
	int iResult = getaddrinfo(host, DCS::Utils::String::From(port).c_str(), &hints, &result);

	SOCKET c_socket = INVALID_SOCKET;

	if (iResult != 0)
	{
		LOG_ERROR("getaddrinfo (WSAAPI) failed: %d", iResult);
		LOG_ERROR("Terminating WSA...");
		WSACleanup();
		return INVALID_SOCKET;
	}

	if (result)
	{
		ptr = result;
		c_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (c_socket == INVALID_SOCKET) {
			LOG_ERROR("socket creation (WSAAPI) failed: %ld", WSAGetLastError());
			LOG_ERROR("Terminating WSA...");
			freeaddrinfo(result);
			WSACleanup();
			return INVALID_SOCKET;
		}

		// Connect to server.
		iResult = connect(c_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			LOG_ERROR("socket connect failed: %ld", WSAGetLastError());
			LOG_ERROR("Closing socket...");
			closesocket(c_socket);
			c_socket = INVALID_SOCKET;
		}

		freeaddrinfo(result);

		if (c_socket == INVALID_SOCKET) {
			LOG_ERROR("Unable to connect to server!");
			LOG_ERROR("Terminating WSA...");
			WSACleanup();
			return INVALID_SOCKET;
		}
	}
	return c_socket;
}

void DCS::Network::ServerListen(SOCKET server)
{
	if (listen(server, SOMAXCONN) == SOCKET_ERROR)
	{
		LOG_ERROR("socket listen (WSAAPI) failed: %ld", WSAGetLastError());
		LOG_ERROR("Closing socket...");
		LOG_ERROR("Terminating WSA...");
		closesocket(server);
		WSACleanup();
	}
}

SOCKET DCS::Network::ServerAcceptConnection(SOCKET server)
{
	SOCKET client = INVALID_SOCKET;
	
	client = accept(server, NULL, NULL);

	if (client == INVALID_SOCKET)
	{
		LOG_ERROR("socket accept (WSAAPI) failed: %d", WSAGetLastError());
		LOG_ERROR("Closing socket...");
		LOG_ERROR("Terminating WSA...");
		closesocket(server);
		WSACleanup();
		return INVALID_SOCKET;
	}

	LOG_WARNING("Established connection with client...");
	/*LOG_WARNING("Closing server socket...");
	closesocket(server);*/

	return client;
}

void DCS::Network::CloseSocketConnection(SOCKET client)
{
	LOG_WARNING("Closing socket...");
	closesocket(client);
}

bool DCS::Network::ValidateSocket(SOCKET s)
{
	return s != INVALID_SOCKET;
}

DCS::i32 DCS::Network::ReceiveData(SOCKET client, unsigned char* buffer, i32 buff_len)
{
	int iResult = recv(client, (char*)buffer, buff_len, 0);

	if (iResult > 0)
	{
		return iResult;
	}
	else if (iResult == 0)
	{
		// Note: Should both connection data flows be shutdown?
		iResult = shutdown(client, SD_BOTH);
		LOG_WARNING("Shuting down socket connection.");
		if (iResult == SOCKET_ERROR)
		{
			LOG_ERROR("socket shutdown failed: %d\n", WSAGetLastError());
			LOG_ERROR("Closing socket...");
			LOG_ERROR("Terminating WSA...");
			closesocket(client);
			WSACleanup();
		}
		return 0;
	}
	else
	{
		//if (WSAGetLastError() != 10054)
		//{
		LOG_ERROR("socket receive failed: %d\n", WSAGetLastError());
		LOG_ERROR("Closing socket...");
		LOG_ERROR("Terminating WSA...");
		closesocket(client);
		WSACleanup();
		//}
		return -1;
	}
}

static DCS::i32 SendGenericData(SOCKET client, const unsigned char* buffer, DCS::i32 buff_len, int flags)
{
	int iSResult = send(client, (const char*)buffer, buff_len, flags);

	if (iSResult > 0)
	{
		return iSResult;
	}
	else
	{
		LOG_ERROR("socket send failed: %d\n", WSAGetLastError());
		LOG_ERROR("Closing socket...");
		LOG_ERROR("Terminating WSA...");
		closesocket(client);
		WSACleanup();
		return -1;
	}
}

DCS::i32 DCS::Network::SendData(SOCKET client, const unsigned char* buffer, i32 buff_len)
{
	return SendGenericData(client, buffer, buff_len, 0);
}

DCS::i32 DCS::Network::SendPriorityData(SOCKET client, const unsigned char* buffer, i32 buff_len)
{
	return SendGenericData(client, buffer, buff_len, MSG_OOB);
}
