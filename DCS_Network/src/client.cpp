#include "../include/DCS_ModuleNetwork.h"
#include "../../DCS_Utils/include/internal.h"
#include "../include/internal.h"
#include <vector>

static std::thread* client_receive_thread = nullptr;
static DCS::Utils::SMessageQueue inbound_data_queue;


static std::thread* client_send_thread = nullptr;
static DCS::Utils::SMessageQueue outbound_data_queue;

static std::atomic<bool> client_running = false;

void DCS::Network::Message::ScheduleTransmission(DefaultMessage msg)
{
	outbound_data_queue.push(msg);
}

DCS::Network::Socket DCS::Network::Client::Connect(DCS::Utils::String host, i32 port)
{
	if (!GetStatus())
	{
		LOG_CRITICAL("Cannot connect to a socket if Network module did not initialize.");
		LOG_CRITICAL("Did you forget to call Network::Init?");
		return (Socket)INVALID_SOCKET;
	}
	else
	{
		SOCKET client = CreateClientSocket(host.c_str(), port);
		return (Socket)client;
	}
}

void DCS::Network::Client::StartThread(Socket connection)
{
	SOCKET target_server = (SOCKET)connection;
	if (ValidateSocket(target_server))
	{
		if (client_receive_thread == nullptr)
		{
			client_running.store(true);
			instances++;
			client_receive_thread = new std::thread([=]()->void {
				unsigned char buffer[512] = { 0 };
				unsigned char* data = new unsigned char[4096]; // TODO : Move this out of here, what if data_sz > 4096 ???
				i32 recv_sz = 1;
				i32 total_size = 0;

				if (data != nullptr)
				{
					while(recv_sz > 0 && client_running.load())
					{
						unsigned char packet_size[sizeof(i32)]; // TODO : Stop assuming size of msg comes in a packet
						i32 resize_sz = 0;

						recv_sz = ReceiveData(target_server, buffer, 512);
						if (recv_sz < 1)
						{
							client_running.store(false);
							delete[] data;
							return;
						}

						memcpy(packet_size, buffer, sizeof(i32));

						i32 msg_size = *(i32*)packet_size;
						LOG_DEBUG("[Client Receive] Expecting %d bytes.", msg_size);

						i32 left = recv_sz - sizeof(i32);
						if (left > 0)
						{
							memcpy(data, buffer + sizeof(i32), left);
							total_size += left;
						}


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
								resize_sz = -msg_size;
							}

							i32 cpy_sz = recv_sz - resize_sz;

							memcpy(data + total_size, buffer, cpy_sz);
							total_size += cpy_sz;
						}

						// TODO : Do something with the data
						LOG_DEBUG("Received value = %d", *(int*)(((DCS::Registry::SVReturn*)(data+1))->ptr));

						total_size = 0;
					}

					if (data != nullptr)
						delete[] data;
					client_running.store(false);
					instances--;
				}
				});
			if (client_receive_thread == nullptr)
			{
				LOG_ERROR("Could not allocate client_receive_thread thread.");
			}
		}
		else
		{
			LOG_WARNING("Could not start client_receive_thread thread. Perhaps is already running?");
		}

		if (client_send_thread == nullptr)
		{
			client_send_thread = new std::thread([=]()->void {
				while (client_running.load())
				{
					auto to_send = outbound_data_queue.pop();
					i32 full_size = (i32)(to_send.size + 1);

					DCS::Network::SendData(target_server, (const u8*)&full_size,  4);
					DCS::Network::SendData(target_server, (const u8*)&to_send.op, 1);
					DCS::Network::SendData(target_server, (const u8*)to_send.ptr, to_send.size);

					Message::Delete(to_send);
				}
			});
			if (client_send_thread == nullptr)
			{
				LOG_ERROR("Could not allocate client_send_thread thread.");
			}
		}
		else
		{
			LOG_WARNING("Could not start client_send_thread thread. Perhaps is already running?");
		}
	}
}

void DCS::Network::Client::StopThread(Socket connection)
{
	if (client_receive_thread != nullptr)
	{
		if (client_running.load())
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
		}

		CloseSocketConnection((SOCKET)connection);

		LOG_DEBUG("Stopping client_receive_thread thread...");

		// Wait for disconnect
		client_receive_thread->join();

		delete client_receive_thread;
	}
	else
	{
		LOG_WARNING("Could not stop client_receive_thread thread. Perhaps is not running?");
	}

	if (client_send_thread != nullptr)
	{
		outbound_data_queue.notify_unblock();
		LOG_DEBUG("Stopping client_send_thread thread...");

		// Wait for disconnect
		client_send_thread->join();

		delete client_send_thread;
	}
	else
	{
		LOG_WARNING("Could not stop client_send_thread thread. Perhaps is not running?");
	}
}
