#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include "../../DCS_Utils/include/internal.h"
#include <vector>
#include <future>

static std::thread* server_receive_thread = nullptr;
static DCS::Utils::SMessageQueue inbound_data_queue;

static std::thread* server_send_thread = nullptr;
static DCS::Utils::SMessageQueue outbound_data_queue;

static std::atomic<bool> server_running = false;

DCS::Network::Socket DCS::Network::Server::Create(i32 port)
{
	if (!GetStatus())
	{
		LOG_CRITICAL("Cannot create a socket if Network module did not initialize.");
		LOG_CRITICAL("Did you forget to call Network::Init?");
		return (Socket)INVALID_SOCKET;
	}
	else
	{
		SOCKET server = CreateServerSocket(port);
		ServerListen(server);
		return (Socket)server;
	}
}

DCS::Network::Socket DCS::Network::Server::WaitForConnection(Socket server)
{
	return (Socket)ServerAcceptConnection((SOCKET)server);
}

void DCS::Network::Server::StartThread(Socket client)
{
	SOCKET target_client = (SOCKET)client;
	if (ValidateSocket(target_client))
	{
		if (server_receive_thread == nullptr)
		{
			server_running.store(true);
			instances++;
			server_receive_thread = new std::thread([=]()->void {
				unsigned char buffer[512] = { 0 };
				unsigned char* data = new unsigned char[4096]; // TODO : Move this out of here, what if data_sz > 4096 ???
				i32 recv_sz = 1;
				i32 total_size = 0;

				if (data != nullptr)
				{
					while (recv_sz > 0 && server_running.load())
					{
						unsigned char packet_size[sizeof(i32)]; // TODO : Stop assuming size of msg comes in a packet
						i32 resize_sz = 0;

						recv_sz = ReceiveData(target_client, buffer, 512);
						if (recv_sz < 1)
						{
							server_running.store(false);
							delete[] data;
							return;
						}

						memcpy(packet_size, buffer, sizeof(i32));

						i32 msg_size = *(i32*)packet_size;
						Message::DefaultMessage msg = Message::Alloc(msg_size);

						i32 left = recv_sz - sizeof(i32);
						if (left > 0)
						{
							memcpy(data, buffer + sizeof(i32), left);
							total_size += left;
						}

						LOG_DEBUG("[Server Receive] Expecting %d bytes.", msg_size);

						msg_size -= left;

						while (msg_size > 0)
						{
							recv_sz = ReceiveData(target_client, buffer, 512);
							if (recv_sz < 1)
							{
								server_running.store(false);
								delete[] data;
								data = nullptr;
								return;
							}

							LOG_DEBUG("Server recv[%d]", recv_sz);
							for (int i = 0; i < recv_sz; i++)
								LOG_DEBUG("0x%02x", buffer[i]);

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

						// Push message to buffer
						Message::Set(msg, data);

						//inbound_data_queue.push(msg);

						// Decide what to do with the data
						switch (static_cast<Message::Operation>(msg.op))
						{
						case DCS::Network::Message::Operation::NO_OP:
							// TODO : Ping back 1 byte
							break;
						case DCS::Network::Message::Operation::REQUEST:
							// For testing remove from here
							DCS::Registry::SVReturn r = DCS::Registry::Execute(DCS::Registry::SVParams::GetParamsFromData(msg.ptr, msg.size));
							LOG_DEBUG("Value: %d", *(int*)r.ptr);
							auto dm = Message::Alloc(1026);
							Message::Set(dm, msg.op, (u8*)&r);
							outbound_data_queue.push(dm);
							break;
						case DCS::Network::Message::Operation::SUB_EVT:
							break;
						case DCS::Network::Message::Operation::UNSUB_EVT:
							break;
						default:
							break;
						}

						Message::Delete(msg); // TODO : Remove this from here
						
						total_size = 0;
					}

					if (data != nullptr)
						delete[] data;
					server_running.store(false);
					instances--;
				}
				});
			if (server_receive_thread == nullptr)
			{
				LOG_ERROR("Could not allocate server_receive_thread thread.");
			}
		}
		else
		{
			LOG_WARNING("Could not start server_receive_thread thread. Perhaps is already running?");
		}

		if (server_send_thread == nullptr)
		{
			server_send_thread = new std::thread([=]()->void {

				while (server_running.load())
				{
					auto to_send = outbound_data_queue.pop();

					i32 full_size = (i32)(to_send.size + 1);
					DCS::Network::SendData(target_client, (const u8*)&full_size, 4);
					DCS::Network::SendData(target_client, (const u8*)&to_send.op, 1);
					DCS::Network::SendData(target_client, (const u8*)to_send.ptr, to_send.size);

					Message::Delete(to_send);
				}
			});
			if (server_send_thread == nullptr)
			{
				LOG_ERROR("Could not allocate server_send_thread thread.");
			}
		}
		else
		{
			LOG_WARNING("Could not start server_send_thread thread. Perhaps is already running?");
		}
	}
}

void DCS::Network::Server::StopThread(Socket client, StopMode mode)
{
	if (server_receive_thread != nullptr)
	{
		if (mode == StopMode::WAIT)
		{
			while (server_running.load())
				std::this_thread::yield();
		}

		CloseSocketConnection((SOCKET)client);

		LOG_DEBUG("Stopping server_receive_thread thread...");

		// Wait for disconnect
		server_receive_thread->join();

		delete server_receive_thread;
	}
	else
	{
		LOG_WARNING("Could not stop server_receive_thread thread. Perhaps is not running?");
	}

	if (server_send_thread != nullptr)
	{
		outbound_data_queue.notify_unblock();
		LOG_DEBUG("Stopping server_send_thread thread...");

		// Wait for disconnect
		server_send_thread->join();

		delete server_send_thread;
	}
	else
	{
		LOG_WARNING("Could not stop server_send_thread thread. Perhaps is not running?");
	}
}
