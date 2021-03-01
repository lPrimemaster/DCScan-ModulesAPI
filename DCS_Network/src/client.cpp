#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include <vector>

static std::thread* client_thread = nullptr;
static std::atomic<bool> client_running = false;


DCS::Network::Socket DCS::Network::Client::Connect(DCS::Utils::String host, i32 port)
{
	if (wsi == nullptr)
	{
		wsi = new WindowsSocketInformation();
		*wsi = InitWinSock();
	}

	SOCKET client = CreateClientSocket(host.c_str(), port);

	return (Socket)client;
}

void DCS::Network::Client::StartThread(Socket connection, OnDataReceivedCallback drc)
{
	SOCKET target_server = (SOCKET)connection;
	if (ValidateSocket(target_server))
	{
		if (client_thread == nullptr)
		{
			client_running.store(true);
			instances++;
			client_thread = new std::thread([=]()->void {
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

						recv_sz = ReceiveData(target_server, buffer, 512);
						if (recv_sz < 1)
						{
							client_running.store(false);
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
							recv_sz = ReceiveData(target_server, buffer, 512);
							if (recv_sz < 1)
							{
								client_running.store(false);
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
						drc(data, total_size, connection);

						total_size = 0;

					} while (recv_sz > 0 || client_running.load());
					instances--;
				}
				});
			if (client_thread == nullptr)
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

void DCS::Network::Client::StopThread(Socket connection)
{
	if (client_thread != nullptr)
	{
		client_running.store(false);

		int iResult = shutdown((SOCKET)connection, SD_SEND);
		LOG_WARNING("Shuting down socket connection.");
		if (iResult == SOCKET_ERROR) {
			LOG_ERROR("socket shutdown failed: %d\n", WSAGetLastError());
			LOG_ERROR("Closing socket...");
			LOG_ERROR("Terminating WSA...");
			closesocket((SOCKET)connection);
			WSACleanup();
			return;
		}

		CloseSocketConnection((SOCKET)connection);

		LOG_DEBUG("Stopping client thread...");

		// Wait for disconnect
		client_thread->join();

		if (wsi != nullptr && instances.load() == 0)
		{
			CleanupWinSock();
			delete wsi;
		}

		delete client_thread;
	}
	else
	{
		LOG_WARNING("Could not stop client thread. Perhaps is not running?");
	}
}

void DCS::Network::Client::SendData(Socket s, const unsigned char* data, DCS::i32 size)
{
	DCS::Network::SendData((SOCKET)s, data, size);
}
