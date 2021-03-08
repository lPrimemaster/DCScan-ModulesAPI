#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Network/include/internal.h"
#include "../../config/registry.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	using namespace DCS::Network;

	Init();

	Socket s = Server::Create(15777);

	std::thread nt([]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		Socket c = Client::Connect("127.0.0.1", 15777);

		Client::StartThread(c);

		unsigned char buffer[1024];
		auto size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Threading_displayFloat,
			1.7f
		);

		Message::SendAsync(Message::Operation::REQUEST, buffer, size_written);

		for (int i = 0; i < 100; i++)
		{
			Message::SendAsync(Message::Operation::REQUEST, buffer, size_written);
		}

		size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
			SV_CALL_DCS_Threading_addInt,
			(int)18,
			(int)19
		);

		for (int i = 0; i < 100; i++)
		{
			auto val = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);
			LOG_MESSAGE("SYNC receive: %d", *(DCS::u16*)val.ptr);
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		Client::StopThread(c);
	});

	Socket client = Server::WaitForConnection(s);

	Server::StartThread(client);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	Server::StopThread(client, Server::StopMode::WAIT);

	nt.join();

	Destroy();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
