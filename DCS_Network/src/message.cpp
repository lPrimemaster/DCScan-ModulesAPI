#include "../include/internal.h"
#include "../DCS_Utils/include/internal.h"

DCS::Network::Message::DefaultMessage DCS::Network::Message::Alloc(i32 size)
{
	DefaultMessage msg;
	msg.size = size - sizeof(msg.op);
	msg.ptr = new u8[msg.size];
	return msg;
}

void DCS::Network::Message::Set(DefaultMessage& msg, u8* data)
{
	if (msg.size > 0)
	{
		msg.op = data[0];
		memcpy(msg.ptr, data + sizeof(msg.op), msg.size);
	}
	else
		LOG_ERROR("Cannot set DefaultMessage: Pointer is null or size is 0.");
}

void DCS::Network::Message::Set(DefaultMessage& msg, u8 opcode, u8* data)
{
	if (msg.size > 0)
	{
		msg.op = opcode;
		memcpy(msg.ptr, data, msg.size);
	}
	else
		LOG_ERROR("Cannot set DefaultMessage: Pointer is null or size is 0.");
}

void DCS::Network::Message::Delete(DefaultMessage msg)
{
	msg.size = 0;
	msg.op = 0; // Message::Operation::NO_OP
	if (msg.ptr != nullptr)
		delete[] msg.ptr;
	else
		LOG_ERROR("Cannot delete DefaultMessage: Pointer is null.");
}

void DCS::Network::Message::SendAsync(Operation op, u8* data, i32 size)
{
	u8 op_code = (u8)DCS::Utils::toUnderlyingType(op);
	u16 total_size = (u16)(size + 1);
	DefaultMessage msg = Message::Alloc(total_size);
	Message::Set(msg, op_code, data);

	ScheduleTransmission(msg);
}
