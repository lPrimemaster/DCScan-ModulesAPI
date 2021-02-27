#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Network/include/internal.h"
#include "../../config/registry.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	using namespace DCS::Network;
	using namespace DCS::Utils;

	Socket s = Server::Create(15777);

	Socket client = Server::WaitForConnection(s);

	Server::StartThread(client, [](const unsigned char* data, DCS::i16 size, Socket client)->void {
		Logger::Debug("Received data: %s", data);
		auto p = DCS::Registry::GetParamsFromData(data, size);

		Logger::Debug("Received values: opcode[%d] fcode[%d] arg0[%d]",
			p.getOpcode(),
			p.getFunccode(),
			p.getArg<DCS::i8>(0)
		);
		});

	Server::StopThread(client, Server::StopMode::WAIT);

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
