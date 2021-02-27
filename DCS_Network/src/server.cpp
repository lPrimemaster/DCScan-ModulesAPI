#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include <vector>
#include <atomic>

static DCS::Network::WindowsSocketInformation* wsi = nullptr;
static std::thread* server_thread = nullptr;
static std::atomic<bool> server_running = false;

using Logger = DCS::Utils::Logger;

DCS::Network::Socket DCS::Network::Server::Create(i32 port)
{
	if (wsi == nullptr)
	{
		wsi = new WindowsSocketInformation();
		*wsi = InitWinSock();
	}
	
	SOCKET server = CreateServerSocket(port);
	ServerListen(server);
	return (Socket)server;
}

DCS::Network::Socket DCS::Network::Server::WaitForConnection(Socket server)
{
	return (Socket)ServerAcceptConnection((SOCKET)server);
}

void DCS::Network::Server::StartThread(Socket client, OnDataReceivedCallback drc)
{
	SOCKET target_client = (SOCKET)client;
	if (ValidateSocket(target_client))
	{
		if (server_thread == nullptr)
		{
			server_running.store(true);
			server_thread = new std::thread([=]()->void {
				std::vector<const unsigned char*> total;
				total.reserve(10);
				unsigned char buffer[512] = { 0 };
				unsigned char* data = new unsigned char[4096];
				size_t recv_sz;
				size_t total_size = 0;

				if (data != nullptr)
				{
					do
					{
						recv_sz = ServerReceiveData(target_client, buffer, 512);
						if (recv_sz < 1) break;

						// Check if data is 0xFF terminated
						if (buffer[recv_sz - 1] == 0xFF)
						{
							memcpy(data + total_size, buffer, recv_sz);
							total_size += recv_sz;

							// Do something with the received data via callback
							drc(data, (i16)total_size, client);

							total_size = 0;
						}
						else
						{
							memcpy(data + total_size, buffer, recv_sz);
							total_size += recv_sz;
						}

					} while (recv_sz > 0);
					server_running.store(false);
					delete[] data;
				}
				});
			if (server_thread == nullptr)
			{
				Logger::Error("Could not allocate server thread.");
			}
		}
		else
		{
			Logger::Warning("Could not start server thread. Perhaps is already running?");
		}
	}
}

void DCS::Network::Server::StopThread(Socket client, StopMode mode)
{
	if (server_thread != nullptr)
	{
		if (mode == StopMode::WAIT)
		{
			while (server_running.load())
				std::this_thread::yield();
		}

		CloseSocketConnection((SOCKET)client);

		Logger::Debug("Stopping server thread...");

		// Wait for disconnect
		server_thread->join();

		if (wsi != nullptr)
		{
			CleanupWinSock();
			delete wsi;
		}

		delete server_thread;
	}
	else
	{
		Logger::Warning("Could not stop server thread. Perhaps is not running?");
	}
}

void DCS::Network::Server::SendData(Socket client, const unsigned char* data, DCS::i16 size)
{
	ServerSendData((SOCKET)client, data, size);
}
