#include "../include/internal.h"

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

void DCS::Network::CleanupWinSock(SOCKET* open_sockets, u16 num_sockets)
{
	for (u16 i = 0; i < num_sockets; i++)
	{
		SOCKET s = open_sockets[i];
		if (s != INVALID_SOCKET)
		{
			Logger::Debug("Closing socket %d of %d...", i, num_sockets);
			closesocket(s);
		}
	}
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

	closesocket(server);

	return client;
}

DCS::i64 DCS::Network::ServerReceiveData(SOCKET client, char* buffer, i16 buff_len)
{
	int iResult = recv(client, buffer, buff_len, 0);

	if (iResult > 0)
	{
		return iResult;
	}
	else if (iResult == 0)
	{
		iResult = shutdown(client, SD_RECEIVE);
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
		Logger::Error("sockect receive failed: %d\n", WSAGetLastError());
		Logger::Error("Closing socket...");
		Logger::Error("Terminating WSA...");
		closesocket(client);
		WSACleanup();
		return -1;
	}
}
