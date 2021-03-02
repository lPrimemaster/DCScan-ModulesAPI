#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include <vector>

static std::thread* server_thread = nullptr;
static std::atomic<bool> server_running = false;

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
			instances++;
			server_thread = new std::thread([=]()->void {
				std::vector<const unsigned char*> total;
				total.reserve(10);
				unsigned char buffer[512] = { 0 };
				unsigned char* data = new unsigned char[4096]; // TODO : Move this out of here, what if data_sz > 4096 ???
				i32 recv_sz;
				i32 total_size = 0;
				

				if (data != nullptr)
				{
					do
					{
						unsigned char packet_size[sizeof(i32)]; // TODO : Stop assuming size of msg comes in a packet
						i32 append_to_start = 0;

						recv_sz = ReceiveData(target_client, buffer, 512);
						if (recv_sz < 1)
						{
							server_running.store(false);
							delete[] data;
							return;
						}

						memcpy(packet_size, buffer, sizeof(i32));

						i32 msg_size = *(i32*)packet_size;

						i32 left = recv_sz - sizeof(i32);
						if (left > 0)
						{
							memcpy(data, buffer + sizeof(i32), left);
							total_size += left;
						}

						LOG_DEBUG("Transmission started: Expecting %d bytes.", msg_size);

						msg_size -= left;

						while (msg_size > 0)
						{
							recv_sz = ReceiveData(target_client, buffer, 512);
							if (recv_sz < 1)
							{
								server_running.store(false);
								delete[] data;
								return;
							}

							msg_size -= recv_sz;

							if (msg_size < 0)
							{
								LOG_WARNING("Message is longer than expected. Discarding...");
								LOG_WARNING("Sending messages too fast is a known bug."
									" If this is the case, consider waiting at least 10 ms between msgs.");
								append_to_start = -msg_size;
							}

							i32 cpy_sz = recv_sz - append_to_start;

							memcpy(data + total_size, buffer, cpy_sz);
							total_size += cpy_sz;
						}

						// TODO : This should not run synchronous for function calls
						// Or maybe it should ??
						drc(data, total_size, client);
						
						total_size = 0;

					} while (recv_sz > 0);
					instances--;
				}
				});
			if (server_thread == nullptr)
			{
				LOG_ERROR("Could not allocate server thread.");
			}
		}
		else
		{
			LOG_WARNING("Could not start server thread. Perhaps is already running?");
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

		LOG_DEBUG("Stopping server thread...");

		// Wait for disconnect
		server_thread->join();

		if (wsi != nullptr && instances.load() == 0)
		{
			CleanupWinSock();
			delete wsi;
		}

		delete server_thread;
	}
	else
	{
		LOG_WARNING("Could not stop server thread. Perhaps is not running?");
	}
}

void DCS::Network::Server::SendData(Socket client, const unsigned char* data, DCS::i32 size)
{
	DCS::Network::SendData((SOCKET)client, data, size);
}