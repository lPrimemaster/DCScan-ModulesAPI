#include "../include/DCS_ModuleNetwork.h"
#include "../include/internal.h"
#include "../../DCS_Utils/include/internal.h"
#include "../../DCS_Core/include/internal.h"
#include <vector>
#include <future>

static std::thread *server_receive_thread = nullptr;
static DCS::Utils::SMessageQueue<DCS::Network::Message::DefaultMessage> inbound_data_queue;

static std::thread *server_send_thread = nullptr;
static DCS::Utils::SMessageQueue<DCS::Network::Message::DefaultMessage> outbound_data_queue;

constexpr DCS::u64 ib_buff_size = 4096 * 1024;
static DCS::Utils::ByteQueue inbound_bytes(ib_buff_size);

static std::thread* server_listen_thread = nullptr;

static std::atomic<bool> server_running = false;
static std::atomic<bool> server_listening = true;
static std::atomic<bool> stop_forced = false;

static std::atomic<DCS::Network::Socket> server_client_sock;
static std::atomic<DCS::Network::Socket> server_listen_sock;


void DCS::Network::Message::FibSeqEvt()
{
	static u64 a = 0;
	static u64 b = 1;

	u64 p = a;

	u64 l = b;
	b = a + b;
	a = l;

	DCS_EMIT_EVT((u8 *)&p, sizeof(u64));
}

void DCS::Network::Message::EmitEvent(u8 EVT_ID, u8 *evtData, i32 size)
{
	// Emit only if event is subscribed to in the client-side
	// NOTE : If this gets too cpu heavy, just consider using a hashmap approach and waking every x millis
	if (DCS::Registry::CheckEvent(EVT_ID))
	{
		// TODO : Allocate via a memory pool (not wasting time with all these new[] operators [via Message::Alloc])
		// Create a Message::PoolAlloc() func
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
		server_listen_sock.store((Socket)server);
		return (Socket)server;
	}
}

void DCS::Network::Server::WaitForConnections(Socket server)
{
	server_listen_thread = new std::thread([=]() -> void {
		std::vector<std::thread> connections;
		while(server_listening.load())
		{
			SOCKET client = ServerAcceptConnection((SOCKET)server);
			connections.push_back(std::thread([&]() {
				if(StartThread((Socket)client))
				{
					StopThread((Socket)client, DCS::Network::Server::StopMode::WAIT);
				}
			}));
		}

		for(auto& c : connections)
		{
			c.join();
		}
	});
}

void DCS::Network::Server::StopListening(Socket server)
{
	LOG_WARNING("Closing server listen socket...");

	SOCKET cts = (SOCKET)server_client_sock.load();
	if(cts != INVALID_SOCKET)
		shutdown(cts, SD_BOTH);
	
	closesocket((SOCKET)server);
	server_listening.store(false);
	server_listen_thread->join();
	delete server_listen_thread;
}

static void SendErrorToClient(SOCKET client, const char* error_msg)
{
	DCS::u8 error = DCS::Utils::toUnderlyingType(DCS::Network::Message::InternalOperation::OP_ERROR);
	DCS::u64 id = 0;
	DCS::i32 str_size = (DCS::i32)strlen(error_msg) + 1;
	DCS::u32 size = (DCS::u32)str_size + MESSAGE_XTRA_SPACE;

	DCS::Network::SendData(client, (const DCS::u8 *)&size, 4);
	DCS::Network::SendData(client, (const DCS::u8 *)&error, 1);
	DCS::Network::SendData(client, (const DCS::u8 *)&id, 8);
	DCS::Network::SendData(client, (const DCS::u8 *)error_msg, str_size);
}

static void IssueValidity(SOCKET client, DCS::u8 validity)
{
	DCS::u8 cval = DCS::Utils::toUnderlyingType(DCS::Network::Message::InternalOperation::CON_VALID);
	DCS::u64 id = 0;
	DCS::i32 cval_size = 1;
	DCS::u32 size = (DCS::u32)cval_size + MESSAGE_XTRA_SPACE;

	DCS::Network::SendData(client, (const DCS::u8 *)&size, 4);
	DCS::Network::SendData(client, (const DCS::u8 *)&cval, 1);
	DCS::Network::SendData(client, (const DCS::u8 *)&id, 8);
	DCS::Network::SendData(client, (const DCS::u8 *)&validity, cval_size);
}

// NOTE : Could also use the tag for auth (?)
static bool Authenticate(SOCKET client)
{
	using namespace DCS;

	// Send user a challenge with a random iv
	Auth::InitCryptoRand();

	u8 r[32];
	Auth::GenerateRandSafeIV128(r);
	Auth::GenerateRandSafeIV128(&r[16]);
	Network::SendData(client, r, 32);

	u8 iv[16];
	Auth::GenerateRandSafeIV128(iv);
	Network::SendData(client, iv, 16);


	u8 username_buff[32] = { 0 };
	i32 username_buff_size = 0;
	while(username_buff_size < 32)
	{
		username_buff_size += Network::ReceiveData(client, &username_buff[username_buff_size], 32 - username_buff_size);
	}

	LOG_DEBUG("Attempt to login with username: %s", (char*)username_buff);

	// Get this user name's password hash
	DB::LoadDefaultDB();
	DB::LoadUsers();
	DB::User user = DB::GetUser((const char*)username_buff);
	if(std::string(user.u) == "INVALID_USER")
	{
		SendErrorToClient(client, "Incorrect username or password.");
		IssueValidity(client, 2); // Not a valid connection

		return false;
	}


	u8 aes_buff[48];
	i32 aes_buff_size = 0;
	while(aes_buff_size < 48)
	{
		aes_buff_size += Network::ReceiveData(client, &aes_buff[aes_buff_size], 48 - aes_buff_size);
	}

	u8 result[32] = { 0 };
	u8 tag[16] = { 0 };
	Auth::EncryptAES256(r, 32, nullptr, 0, user.p, iv, result, tag);

	for(int i = 0; i < 32; i++)
	{
		if(result[i] != aes_buff[i])
		{
			SendErrorToClient(client, "Incorrect username or password.");
			IssueValidity(client, 2); // Not a valid connection
			return false;
		}
	}

	return true;
}

bool DCS::Network::Server::StartThread(Socket client)
{
	SOCKET target_client = (SOCKET)client;
	if (ValidateSocket(target_client))
	{
		if (server_receive_thread == nullptr)
		{
			if(!Authenticate((SOCKET)client))
			{
				LOG_WARNING("User authentication failed.");

				char ip[128];
				GetSocketIpAddress((SOCKET)client, ip);
				LOG_WARNING("Failed at ip address: %s", ip);

				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				shutdown((SOCKET)client, SD_BOTH);
				CloseSocketConnection((SOCKET)client);

				return false;
			}
			else
			{
				char ip[128];
				GetSocketIpAddress((SOCKET)client, ip);
				LOG_DEBUG("User authentication successful. [%s]", ip);
			}

			server_client_sock.store(client);
			server_running.store(true);
			
			LOG_DEBUG("Starting server_receive_thread thread...");

			std::thread *decode_msg = new std::thread([=]() -> void {
				unsigned char* buffer = new unsigned char[ib_buff_size];

				while (server_running.load())
				{
					u64 sz = inbound_bytes.fetchNextMsg(buffer);

					if (sz == 0)
						continue; // Guard against inbound_bytes notify_unblock

					auto msg = Message::Alloc((i32)sz);

					// Push message to buffer
					Message::SetCopyIdAndCode(msg, buffer);

					LOG_DEBUG("Server received opcode: %u", msg.op);

					// Decide what to do with the data
					switch (static_cast<Message::InternalOperation>(msg.op))
					{
					case DCS::Network::Message::InternalOperation::NO_OP:
					{
						// FIXME : Keepalive 10 sec ping
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
						// Execute request locally
						DCS::Registry::SVReturn r = DCS::Registry::Execute(DCS::Registry::SVParams::GetParamsFromData(msg.ptr, (i32)msg.size));

						// Send the return value if it exists
						// Send the return value (even if SV_RET_VOID)
						auto dm = Message::Alloc(sizeof(r) + MESSAGE_XTRA_SPACE);
						Message::SetCopyId(dm,
											DCS::Utils::toUnderlyingType(Message::InternalOperation::ASYNC_RESPONSE),
											msg.id,
											(u8 *)&r);
						outbound_data_queue.push(dm);
					}
					break;
					case DCS::Network::Message::InternalOperation::SYNC_REQUEST:
					{
						// Execute request locally
						DCS::Registry::SVReturn r = DCS::Registry::Execute(DCS::Registry::SVParams::GetParamsFromData(msg.ptr, (i32)msg.size));

						// Send the return value (even if SV_RET_VOID)
						auto dm = Message::Alloc(sizeof(r) + MESSAGE_XTRA_SPACE);
						Message::SetCopyId(dm,
										   DCS::Utils::toUnderlyingType(Message::InternalOperation::SYNC_RESPONSE),
										   msg.id,
										   (u8 *)&r);
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
				delete[] buffer;
				inbound_bytes.notify_restart();
			});

			server_receive_thread = new std::thread([=]() -> void {
				constexpr u64 buff_size = 512;
				unsigned char buffer[buff_size] = {0};
				i32 recv_sz = 1;

				while (recv_sz > 0 && server_running.load())
				{
					recv_sz = ReceiveData(target_client, buffer, buff_size);

					if (recv_sz > 0)
						inbound_bytes.addBytes(buffer, recv_sz);
				}

				server_running.store(false);
				server_client_sock.store((Socket)INVALID_SOCKET);
				inbound_bytes.notify_unblock();
				decode_msg->join();
				delete decode_msg;
			});
			if (server_receive_thread == nullptr)
			{
				LOG_ERROR("Could not allocate server_receive_thread thread.");
			}

			IssueValidity(target_client, 1); // Valid
		}
		else
		{
			LOG_WARNING("Could not start server_receive_thread thread. Perhaps is already running?");

			SendErrorToClient(target_client, "Connection is not unique. Closing connection.");

			char ip[128];
			GetSocketIpAddress((SOCKET)GetConnectedClient(), ip);

			char msg_buf[512];
			sprintf(msg_buf, "Wait for user at %s to disconnect and try again.", ip);
			SendErrorToClient(target_client, msg_buf);

			IssueValidity(target_client, 2); // Not valid
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			shutdown(target_client, SD_BOTH);

			CloseSocketConnection(target_client);
			return false;
		}

		if (server_send_thread == nullptr)
		{
			LOG_DEBUG("Starting server_send_thread thread...");

			server_send_thread = new std::thread([=]() -> void {
				while (server_running.load())
				{
					auto to_send = outbound_data_queue.pop();

					// To protect SendData against notify_unblock()
					if (to_send.ptr != nullptr)
					{
						i32 full_size = (i32)(to_send.size + MESSAGE_XTRA_SPACE);
						DCS::Network::SendData(target_client, (const u8 *)&full_size, 4);
						DCS::Network::SendData(target_client, (const u8 *)&to_send.op, 1);
						DCS::Network::SendData(target_client, (const u8 *)&to_send.id, 8);
						DCS::Network::SendData(target_client, (const u8 *)to_send.ptr, (i32)to_send.size);
						Message::Delete(to_send);
					}
				}
			});
			if (server_send_thread == nullptr)
			{
				LOG_ERROR("Could not allocate server_send_thread thread.");
			}
			return true;
		}
		else
		{
			LOG_WARNING("Could not start server_send_thread thread. Perhaps is already running?");
			return false;
		}
	}
	return false;
}

void DCS::Network::Server::StopThread(Socket client, StopMode mode)
{
	if (server_receive_thread != nullptr)
	{
		if (mode == StopMode::WAIT)
		{
			while (server_running.load())
				std::this_thread::yield();
				
			if (stop_forced.load())
				return;
		}
		else
		{
			stop_forced.store(true);
		}

		CloseSocketConnection((SOCKET)client);

		LOG_DEBUG("Stopping server_receive_thread thread...");

		// Wait for disconnect
		server_receive_thread->join();

		delete server_receive_thread;
		server_receive_thread = nullptr;
		LOG_DEBUG("Done!");
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
		server_send_thread = nullptr;
		LOG_DEBUG("Done!");
	}
	else
	{
		LOG_WARNING("Could not stop server_send_thread thread. Perhaps is not running?");
	}
}

DCS::Network::Socket DCS::Network::Server::GetConnectedClient()
{
	return server_client_sock.load();
}

DCS::Network::Socket DCS::Network::Server::GetListenSocket()
{
	return server_listen_sock.load();
}

bool DCS::Network::Server::IsRunning()
{
	return server_running.load();
}
