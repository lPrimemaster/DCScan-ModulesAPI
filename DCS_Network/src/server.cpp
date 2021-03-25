#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include "../../DCS_Utils/include/internal.h"
#include <vector>
#include <future>

static std::thread* server_receive_thread = nullptr;
static DCS::Utils::SMessageQueue inbound_data_queue;

static std::thread* server_send_thread = nullptr;
static DCS::Utils::SMessageQueue outbound_data_queue;

constexpr DCS::u64 ib_buff_size = 4096;
static DCS::Utils::ByteQueue inbound_bytes(ib_buff_size);

static std::atomic<bool> server_running = false;

void DCS::Network::Message::FibSeqEvt()
{
	static u64 a = 0;
	static u64 b = 1;

	u64 p = a;
	
	u64 l = b;
	b = a + b;
	a = l;
	
	DCS_EMIT_EVT(SV_EVT_OnTestFibSeq, (u8*)&p, sizeof(u64));
}

void DCS::Network::Message::EmitEvent(u8 EVT_ID, u8* evtData, i32 size)
{
	// Emit only if event is subscribed to in the client-side
	// NOTE : If this gets too cpu heavy, just consider using a hashmap approach and waking every x millis
	if (DCS::Registry::CheckEvent(EVT_ID))
	{
		// TODO : Allocate via a memory pool (not wasting time with all these new[] operators [via Message::Alloc])
		// TODO : Create a Message::PoolAlloc() func
		DefaultMessage msg = Message::Alloc(size + sizeof(u8) + MESSAGE_XTRA_SPACE);

		*msg.ptr = EVT_ID;
		
		// memcpy performance is fine here
		memcpy(msg.ptr + 1, evtData, size);

		Message::SetNew(msg, DCS::Utils::toUnderlyingType(Message::InternalOperation::EVT_RESPONSE), nullptr);

		outbound_data_queue.push(msg);
	}
}

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

			std::thread* decode_msg = new std::thread([=]()->void {
				
				unsigned char buffer[ib_buff_size] = { 0 };

				while (server_running.load() || inbound_bytes.count() > 0)
				{
					u64 sz = inbound_bytes.fetchNextMsg(buffer);

					if (sz == 0) continue; // Guard against inbound_bytes notify_unblock
					
					auto msg = Message::Alloc((i32)sz);

					// Push message to buffer
					Message::SetCopyIdAndCode(msg, buffer);

					LOG_DEBUG("Server received opcode: %u", msg.op);

					// Decide what to do with the data
					switch (static_cast<Message::InternalOperation>(msg.op))
					{
					case DCS::Network::Message::InternalOperation::NO_OP:
					{
						//// Ping back 1 byte Used for latency check
						//u8 dd = 0x0;
						//auto dm = Message::Alloc(1 + MESSAGE_XTRA_SPACE);
						//Message::SetCopyId(dm,
						//	DCS::Utils::toUnderlyingType(Message::InternalOperation::NO_OP),
						//	msg.id, &dd);
						//outbound_data_queue.push(dm);
					}
					break;
					case DCS::Network::Message::InternalOperation::ASYNC_REQUEST:
					{
						LOG_DEBUG("Payload size: %d", msg.size);
						// Execute request locally
						DCS::Registry::SVReturn r = DCS::Registry::Execute(DCS::Registry::SVParams::GetParamsFromData(msg.ptr, (i32)msg.size));

						// Send the return value if it exists
						if (r.type != SV_RET_VOID)
						{
							auto dm = Message::Alloc(sizeof(r) + MESSAGE_XTRA_SPACE);
							Message::SetCopyId(dm,
								DCS::Utils::toUnderlyingType(Message::InternalOperation::ASYNC_RESPONSE), 
								msg.id, 
								(u8*)&r);
							outbound_data_queue.push(dm);
						}
					}
					break;
					case DCS::Network::Message::InternalOperation::SYNC_REQUEST:
					{
						LOG_DEBUG("Payload size: %d", msg.size);
						// Execute request locally
						DCS::Registry::SVReturn r = DCS::Registry::Execute(DCS::Registry::SVParams::GetParamsFromData(msg.ptr, (i32)msg.size));

						// Send the return value (even if SV_RET_VOID)
						auto dm = Message::Alloc(sizeof(r) + MESSAGE_XTRA_SPACE);
						Message::SetCopyId(dm, 
							DCS::Utils::toUnderlyingType(Message::InternalOperation::SYNC_RESPONSE), 
							msg.id,
							(u8*)&r);
						outbound_data_queue.push(dm);
					}
					break;
					case DCS::Network::Message::InternalOperation::EVT_SUB:

						LOG_MESSAGE("Subscribing to event id: %u", *msg.ptr);
						DCS::Registry::SetEvent(*msg.ptr);

						break;
					case DCS::Network::Message::InternalOperation::EVT_UNSUB:

						LOG_MESSAGE("Unsubscribing from event id: %u", *msg.ptr);
						DCS::Registry::UnsetEvent(*msg.ptr);

						break;
					default:
						break;
					}

					Message::Delete(msg);
				}
			});

			server_receive_thread = new std::thread([=]()->void {
				constexpr u64 buff_size = 512;
				unsigned char buffer[buff_size] = { 0 };
				i32 recv_sz = 1;

				while (recv_sz > 0 && server_running.load())
				{
					recv_sz = ReceiveData(target_client, buffer, buff_size);

					LOG_DEBUG("Recv size: %d", recv_sz);

					if (recv_sz > 0) inbound_bytes.addBytes(buffer, recv_sz);
				}
				server_running.store(false);
				inbound_bytes.notify_unblock();
				decode_msg->join();
				delete decode_msg;
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

					// To protect SendData against notify_unblock()
					if (to_send.ptr != nullptr)
					{
						i32 full_size = (i32)(to_send.size + MESSAGE_XTRA_SPACE);
						DCS::Network::SendData(target_client, (const u8*)&full_size, 4);
						DCS::Network::SendData(target_client, (const u8*)&to_send.op, 1);
						DCS::Network::SendData(target_client, (const u8*)&to_send.id, 8);
						DCS::Network::SendData(target_client, (const u8*)to_send.ptr, (i32)to_send.size);
						Message::Delete(to_send);
					}
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
