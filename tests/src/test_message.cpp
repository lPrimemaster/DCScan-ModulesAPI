#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Network/include/internal.h"
#include "../../config/registry.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	using namespace DCS::Network;

	Socket s = Server::Create(15777);

	std::thread nt([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		Socket c = Client::Connect("127.0.0.1", 15777);

		Client::SendData(c, (const unsigned char*)"\x05\x00\x00\x00", 4);
		Client::SendData(c, (const unsigned char*)"\x03", 1);
		Client::SendData(c, (const unsigned char*)"\x01\x00", 2);
		Client::SendData(c, (const unsigned char*)"\x00\xFF", 2);

		Client::StartThread(c, [](const unsigned char* data, DCS::i32 size, Socket client)->void {
				LOG_DEBUG("[Client] Received data: %s", data);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(3000));

		Client::StopThread(c);
	});

	Socket client = Server::WaitForConnection(s);

	Server::StartThread(client, [](const unsigned char* data, DCS::i32 size, Socket client)->void {
		LOG_DEBUG("[Server] Received data: %s", data);
		auto p = DCS::Registry::GetParamsFromData(data, size);

		LOG_DEBUG("Received values: opcode[%d] fcode[%d] arg0[%d]",
			p.getOpcode(),
			p.getFunccode(),
			p.getArg<DCS::i8>(0)
		);

		if (p.getOpcode() != 2)
		{
			auto r = DCS::Registry::Execute(p).cast<DCS::u16>();

			Server::SendData(client, (unsigned char*)"\x02\x00\x00\x00", 4);
			Server::SendData(client, (unsigned char*)&r, sizeof(DCS::u16));
		}
	});

	Server::StopThread(client, Server::StopMode::WAIT);

	nt.join();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
