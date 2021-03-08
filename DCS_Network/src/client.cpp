#include "../include/DCS_ModuleNetwork.h"
#include "../../DCS_Utils/include/internal.h"
#include "../include/internal.h"

#include <vector>

static std::thread* client_receive_thread = nullptr;
static DCS::Utils::SMessageQueue inbound_data_queue;


static std::thread* client_send_thread = nullptr;
static DCS::Utils::SMessageQueue outbound_data_queue;

static DCS::Utils::ByteQueue inbound_bytes(4096);

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

			std::thread* decode_msg = new std::thread([=]()->void {
				unsigned char buffer[4096] = { 0 };

				while (client_running.load() || inbound_bytes.count() > 0)
				{
					u64 sz = inbound_bytes.fetchNextMsg(buffer);

					auto msg = Message::Alloc(sz);

					// Push message to buffer
					Message::SetCopyIdAndCode(msg, buffer);

					// Decide what to do with the data
					switch (static_cast<Message::InternalOperation>(msg.op))
					{
					case DCS::Network::Message::InternalOperation::NO_OP:
						// TODO : Ping back 1 byte
						break;
					case DCS::Network::Message::InternalOperation::ASYNC_RESPONSE:
					{
						u16 rvalue = *(u16*)(((DCS::Registry::SVReturn*)msg.ptr)->ptr);
						LOG_DEBUG("Client Value ASYNC: %d", rvalue);

						// TODO : Call client user defined callback for async calls
					}
					break;
					case DCS::Network::Message::InternalOperation::SYNC_RESPONSE:
					{
						DCS::Network::Message::SetMsgIdCondition(msg);
					}
					break;
					case DCS::Network::Message::InternalOperation::SUB_EVT:
						break;
					case DCS::Network::Message::InternalOperation::UNSUB_EVT:
						break;
					default:
						break;
					}

					Message::Delete(msg);
				}
				});

			client_receive_thread = new std::thread([=]()->void {
				unsigned char buffer[512] = { 0 };
				i32 recv_sz = 1;

				while (recv_sz > 0 && client_running.load())
				{
					recv_sz = ReceiveData(target_server, buffer, 512);

					if (recv_sz > 0) inbound_bytes.addBytes(buffer, recv_sz);
				}
				client_running.store(false);
				decode_msg->join();
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

					// To protect SendData against notify_unblock()
					if (to_send.ptr != nullptr)
					{
						i32 full_size = (i32)(to_send.size + MESSAGE_XTRA_SPACE);
						DCS::Network::SendData(target_server, (const u8*)&full_size, 4);
						DCS::Network::SendData(target_server, (const u8*)&to_send.op, 1);
						DCS::Network::SendData(target_server, (const u8*)&to_send.id, 8);
						DCS::Network::SendData(target_server, (const u8*)to_send.ptr, (i32)to_send.size);
						Message::Delete(to_send);
						/*LOG_DEBUG("Sent message");
						LOG_DEBUG("%d", full_size);
						LOG_DEBUG("%d", to_send.op);
						LOG_DEBUG("%d", to_send.id);
						LOG_DEBUG("%d", to_send.size);*/
					}
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
