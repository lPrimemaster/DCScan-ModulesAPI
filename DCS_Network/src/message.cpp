#include "../include/internal.h"
#include "../DCS_Utils/include/internal.h"

std::mutex DCS::Network::Message::message_m;
std::condition_variable DCS::Network::Message::lsync;
DCS::Network::Message::DefaultMessage DCS::Network::Message::lmessage;
std::map<DCS::u64, std::promise<DCS::Registry::SVReturn>> async_mess_prom;

DCS::Registry::SVReturn DCS::Network::Message::WaitForId(DCS::u64 id)
{
	std::unique_lock<std::mutex> lock(message_m);
	//LOG_DEBUG("SendSync waiting... [%d][%d]", id, lmessage.id);
	lsync.wait(lock, [&] { return lmessage.ptr != nullptr && lmessage.id == id; }); // This makes WaitForId thread-safe
	//LOG_DEBUG("SendSync resumed...");
	//LOG_DEBUG("SendSync got %d", *(u16*)((DCS::Registry::SVReturn*)lmessage.ptr)->ptr);

	Registry::SVReturn ret;
	auto msg_ptr = lmessage.ptr;
	auto msg_sz = lmessage.size;
	auto rp = (Registry::SVReturn*)msg_ptr;
	memcpy(&ret, rp, msg_sz);

	Delete(lmessage);

	lock.unlock();
	return ret;
}

void DCS::Network::Message::SetMsgIdCondition(DefaultMessage& msg)
{
	std::unique_lock<std::mutex> lock(message_m);

	if (lmessage.ptr != nullptr)
	{
		Delete(lmessage);
	}

	lmessage = Copy(msg);

	lock.unlock();

	lsync.notify_all();
}

void DCS::Network::Message::NotifyPromise(DefaultMessage& msg)
{
	async_mess_prom.at(msg.id).set_value(*(DCS::Registry::SVReturn*)msg.ptr);
	async_mess_prom.erase(msg.id);
}

DCS::Network::Message::DefaultMessage DCS::Network::Message::Alloc(i32 size)
{
	DefaultMessage msg;
	msg.size = size - sizeof(msg.op) - sizeof(msg.id);
	msg.ptr = new u8[msg.size];
	return msg;
}

void DCS::Network::Message::SetCopyIdAndCode(DefaultMessage& msg, u8* data)
{
	if (msg.size > 0)
	{
		msg.op = data[0];
		msg.id = *((u64*)(data+1));
		memcpy(msg.ptr, data + sizeof(msg.op) + sizeof(msg.id), msg.size);
	}
	else
		LOG_ERROR("Cannot set DefaultMessage: Pointer is null or size is 0.");
}

void DCS::Network::Message::SetCopyId(DefaultMessage& msg, u8 opcode, u64 id, u8* data)
{
	if (msg.size > 0)
	{
		msg.op = opcode;
		msg.id = id;
		memcpy(msg.ptr, data, msg.size);
	}
	else
		LOG_ERROR("Cannot set DefaultMessage: Pointer is null or size is 0.");
}

void DCS::Network::Message::SetNew(DefaultMessage& msg, u8 opcode, u8* data)
{
	static std::atomic<u64> nid = 0;
	if (msg.size > 0)
	{
		msg.op = opcode;
		msg.id = nid++;

		if (data != nullptr)
			memcpy(msg.ptr, data, msg.size);
	}
	else
		LOG_ERROR("Cannot set DefaultMessage: Pointer is null or size is 0.");
}

DCS::Network::Message::DefaultMessage DCS::Network::Message::Copy(DefaultMessage& msg)
{
	DefaultMessage cpy;

	cpy.ptr = nullptr;
	cpy.id = msg.id;
	cpy.op = msg.op;
	cpy.size = msg.size;

	if (msg.size > 0)
	{
		cpy.ptr = new u8[msg.size];

		memcpy(cpy.ptr, msg.ptr, cpy.size);
	}
	else
		LOG_ERROR("Cannot set DefaultMessage: Pointer is null or size is 0.");

	return cpy;
}

void DCS::Network::Message::Delete(DefaultMessage& msg)
{
	msg.size = 0;
	msg.op = 0;
	if (msg.ptr != nullptr)
	{
		delete[] msg.ptr;
		msg.ptr = nullptr;
	}
	else
		LOG_ERROR("Cannot delete DefaultMessage: Pointer is null.");
}

DCS::Utils::AsyncItem<DCS::Registry::SVReturn> DCS::Network::Message::SendAsync(Operation op, u8* data, i32 size)
{
	std::promise<DCS::Registry::SVReturn> p;
	DCS::Utils::AsyncItem<DCS::Registry::SVReturn> ret(p);

	u8 xtra_op = 0;
	if (op == DCS::Network::Message::Operation::REQUEST)
	{
		xtra_op = 2; // InternalOp Async_req (3)
	}

	u8 op_code = (u8)(DCS::Utils::toUnderlyingType(op) + xtra_op);
	u16 total_size = (u16)(size + MESSAGE_XTRA_SPACE);
	DefaultMessage msg = Message::Alloc(total_size);
	Message::SetNew(msg, op_code, data);

	ScheduleTransmission(msg);

	async_mess_prom.emplace(msg.id, std::move(p));

	return ret;
}

DCS::Registry::SVReturn DCS::Network::Message::SendSync(Operation op, u8* data, i32 size)
{
	Registry::SVReturn ret;
	ret.type = SV_RET_VOID;

	u8 xtra_op = 0;
	if (op == DCS::Network::Message::Operation::REQUEST)
	{
		xtra_op = 1; // InternalOp Sync_req (2)
	}

	u8 op_code = (u8)(DCS::Utils::toUnderlyingType(op) + xtra_op);
	u16 total_size = (u16)(size + MESSAGE_XTRA_SPACE);
	DefaultMessage msg = Message::Alloc(total_size);
	Message::SetNew(msg, op_code, data);

	ScheduleTransmission(msg);

	ret = WaitForId(msg.id);
	return ret;
}
