#include "../include/DCS_ModuleNetwork.h"
#include "../../DCS_Utils/include/internal.h"
#include "../include/internal.h"

#include <vector>
#include <future>

static std::thread* client_receive_thread = nullptr;
static DCS::Utils::SMessageQueue inbound_data_queue;


static std::thread* client_send_thread = nullptr;
static DCS::Utils::SMessageQueue outbound_data_queue;

static DCS::Utils::ByteQueue inbound_bytes(4096);

static std::atomic<bool> client_running = false;
static std::atomic<DCS::i16> server_latency_ms = 0;
static std::atomic<DCS::u8> conn_valid = false;

#define C_WAIT 0
#define C_VALID 1
#define C_INVALID 2

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

bool DCS::Network::Client::StartThread(Socket connection)
{
	SOCKET target_server = (SOCKET)connection;
	if (ValidateSocket(target_server))
	{
		if (client_receive_thread == nullptr)
		{
			client_running.store(true);

			std::thread* decode_msg = new std::thread([=]()->void {
				unsigned char buffer[4096] = { 0 };
				
				// Start server heartbeat
				/*DCS::Timer::SystemTimer timer = DCS::Timer::New();
				DCS::Timer::Timestamp ts;
				std::future<void> nblock;

				u8 dd = 0x0;
				Message::SendAsync(Message::Operation::NO_OP, &dd, 1);
				ts = DCS::Timer::GetTimestamp(timer);*/

				while (client_running.load() || inbound_bytes.count() > 0)
				{
					u64 sz = inbound_bytes.fetchNextMsg(buffer);

					if (sz == 0) continue; // Guard against inbound_bytes notify_unblock

					auto msg = Message::Alloc((i32)sz);

					// Push message to buffer
					Message::SetCopyIdAndCode(msg, buffer);

					// Decide what to do with the data
					switch (static_cast<Message::InternalOperation>(msg.op))
					{
					case DCS::Network::Message::InternalOperation::NO_OP:
					{
						// Latency check update every 10 sec
						//auto now = DCS::Timer::GetTimestamp(timer);
						//server_latency_ms.store((now.millis - ts.millis + (now.sec - ts.sec) * 1000));

						//// BUG : If server disconnect happens at same time of keepalive (Message::SendAsync still runs)
						//// causing socket send error
						//nblock = std::async(std::launch::async, [&]() {
						//	// Heartbeat for 10 seconds to keepalive
						//	std::this_thread::sleep_for(std::chrono::seconds{ 10 });
						//	if (client_running.load())
						//	{
						//		u8 dd = 0x0;
						//		Message::SendAsync(Message::Operation::NO_OP, &dd, 1);
						//		ts = DCS::Timer::GetTimestamp(timer);
						//	}
						//});
					}
					break;
					case DCS::Network::Message::InternalOperation::ASYNC_RESPONSE:
					{
						u16 rvalue = *(u16*)(((DCS::Registry::SVReturn*)msg.ptr)->ptr);
						LOG_DEBUG("Client Value ASYNC: %d", rvalue);

						// TODO : Wrap around std::future for this
					}
					break;
					case DCS::Network::Message::InternalOperation::SYNC_RESPONSE:
					{
						// Set last message (break out of WaitForId)
						DCS::Network::Message::SetMsgIdCondition(msg);
					}
					break;
					case DCS::Network::Message::InternalOperation::EVT_RESPONSE:

						// Call client-side user callback
						DCS::Registry::GetEventCallback(*(u8*)msg.ptr)(msg.ptr + 1);

						break;
					case DCS::Network::Message::InternalOperation::EVT_UNSUB:
						break;
					case DCS::Network::Message::InternalOperation::OP_ERROR:
					{
						// NOTE : Setup error codes rather then only strings??
						const char* err_msg = (const char*)msg.ptr;
						LOG_ERROR("Server responded with error: %s", err_msg);
					}
					break;
					case DCS::Network::Message::InternalOperation::CON_VALID:
					{
						if(*msg.ptr == C_VALID)
						{
							LOG_DEBUG("Server connection is VALID.");
							conn_valid.store(C_VALID);
						}
						else
						{
							LOG_DEBUG("Server connection is INVALID.");
					        conn_valid.store(C_INVALID); // Connection not valid
							client_running.store(false); // Stop the client
							inbound_bytes.notify_unblock();
						}
					}
					break;
					default:
						break;
					}

					Message::Delete(msg);
				}
				//DCS::Timer::Delete(timer);
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
				conn_valid.store(C_WAIT);
				inbound_bytes.notify_unblock();
				decode_msg->join();
				delete decode_msg;
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
			// This is just a very cheap way... Just use std::condition_variable...
			while(conn_valid.load() == 0) std::this_thread::yield();

			if(conn_valid.load() == 2)
				return false;

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
		return true;
	}
	return false;
}

void DCS::Network::Client::StopThread(Socket connection)
{
	if (client_receive_thread != nullptr)
	{
		if (client_running.load())
		{
			client_running.store(false);

			int iResult = shutdown((SOCKET)connection, SD_SEND);
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

DCS::i16 DCS::Network::Client::GetMillisLatency()
{
	return server_latency_ms.load();
}
