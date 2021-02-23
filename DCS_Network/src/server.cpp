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
				std::vector<std::string> total;
				char buffer[512] = { 0 };
				size_t recv_sz;
				u16 offset;

				do
				{
					recv_sz = ServerReceiveData(target_client, buffer, 512);
					if (recv_sz < 1) break;

					// Check if data is null terminated
					if (buffer[recv_sz - 1] == '\0') offset = 1; else offset = 0;

					//Check if data ended via token -> \r\n
					if (buffer[recv_sz - offset - 1] == '\n' && buffer[recv_sz - offset - 2] == '\r')
					{
						total.push_back(std::string(buffer, recv_sz - offset - 2)); // Ignore the \r\n token
						std::string full_message = "";
						for (auto str : total)
						{
							full_message += str;
						}

						// Do something with the received data via callback
						drc(full_message.c_str(), full_message.size(), client);

						total.clear();
					}
					else
					{
						total.push_back(std::string(buffer, recv_sz)); // Data might not be null terminated
					}

				} while (recv_sz > 0);
				server_running.store(false);
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

void DCS::Network::Server::StopThread(Socket client)
{
	if (server_thread != nullptr)
	{
		if (server_running.load())
		{
			CloseSocketConnection((SOCKET)client);
		}

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

void DCS::Network::Server::SendData(Socket client, const char* data, DCS::i64 size)
{

}
