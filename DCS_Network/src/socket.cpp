#include "../include/internal.h"

using Logger = DCS::Utils::Logger;

DCS::Network::WindowsSocketInformation DCS::Network::InitWinSock()
{
	WindowsSocketInformation wsi;
	
	int iResult = WSAStartup(MAKEWORD(2,2), &(wsi.wsa));

	if (iResult != 0)
	{
		Logger::Error("Windows socket implementation (WSA) failed: %d", iResult);
	}
	else
	{
		Logger::Debug("Windows socket implementation (WSA) initialized.");
	}
	return wsi;
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
		Logger::Error("getaddrinfo (WSAAPI) failed: %d", iResult);
		Logger::Error("Terminating WSA...");
		WSACleanup();
		return INVALID_SOCKET;
	}
	Logger::Debug("IP Address found.");

	SOCKET ListenSocket = INVALID_SOCKET;

	// Create the socket
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET)
	{
		Logger::Error("socket creation (WSAAPI) failed: %ld", WSAGetLastError());
		Logger::Error("Terminating WSA...");
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}
	Logger::Debug("IPv4 socket created.");

	// Bind the socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	if (iResult == SOCKET_ERROR)
	{
		Logger::Error("socket bind (WSAAPI) failed: %d", WSAGetLastError());
		Logger::Error("Closing socket...");
		Logger::Error("Terminating WSA...");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}
	Logger::Debug("Socket bind successful.");
	freeaddrinfo(result);

	return ListenSocket;
}

void DCS::Network::ServerListen(SOCKET server)
{
	if (listen(server, SOMAXCONN) == SOCKET_ERROR)
	{
		Logger::Error("socket listen (WSAAPI) failed: %ld", WSAGetLastError());
		Logger::Error("Closing socket...");
		Logger::Error("Terminating WSA...");
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
		Logger::Error("socket accept (WSAAPI) failed: %d", WSAGetLastError());
		Logger::Error("Closing socket...");
		Logger::Error("Terminating WSA...");
		closesocket(server);
		WSACleanup();
		return INVALID_SOCKET;
	}

	Logger::Warning("Established connection with client...");
	Logger::Warning("Closing server socket...");
	closesocket(server);

	return client;
}

void DCS::Network::CloseSocketConnection(SOCKET client)
{
	Logger::Warning("Closing socket...");
	closesocket(client);
}

bool DCS::Network::ValidateSocket(SOCKET s)
{
	return s != INVALID_SOCKET;
}

DCS::i64 DCS::Network::ServerReceiveData(SOCKET client, unsigned char* buffer, i16 buff_len)
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
		Logger::Warning("Shuting down socket connection.");
		if (iResult == SOCKET_ERROR)
		{
			Logger::Error("socket shutdown failed: %d\n", WSAGetLastError());
			Logger::Error("Closing socket...");
			Logger::Error("Terminating WSA...");
			closesocket(client);
			WSACleanup();
		}
		return 0;
	}
	else
	{
		Logger::Error("socket receive failed: %d\n", WSAGetLastError());
		Logger::Error("Closing socket...");
		Logger::Error("Terminating WSA...");
		closesocket(client);
		WSACleanup();
		return -1;
	}
}

static DCS::i64 SendGenericData(SOCKET client, const unsigned char* buffer, DCS::i16 buff_len, int flags)
{
	int iSResult = send(client, (const char*)buffer, buff_len, flags);

	if (iSResult > 0)
	{
		return iSResult;
	}
	else
	{
		Logger::Error("socket send failed: %d\n", WSAGetLastError());
		Logger::Error("Closing socket...");
		Logger::Error("Terminating WSA...");
		closesocket(client);
		WSACleanup();
		return -1;
	}
}

DCS::i64 DCS::Network::ServerSendData(SOCKET client, const unsigned char* buffer, i16 buff_len)
{
	return SendGenericData(client, buffer, buff_len, 0);
}

DCS::i64 DCS::Network::ServerSendPriorityData(SOCKET client, const unsigned char* buffer, i16 buff_len)
{
	return SendGenericData(client, buffer, buff_len, MSG_OOB);
}
