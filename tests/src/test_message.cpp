#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Network/include/internal.h"
#include "../../config/registry.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	using namespace DCS::Network;

	Init();

	//Socket s = Server::Create(15777);

	std::thread nt([&]() { //ref capture for DCS_START_TEST
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		Socket c = Client::Connect("127.0.0.1", 15777);

		Client::Authenticate(c, "Prime", "alfa77");

		if(Client::StartThread(c))
		{
			unsigned char buffer[1024];
			auto size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
				SV_CALL_DCS_Threading_GetMaxHardwareConcurrency
			);

			auto task = Message::SendAsync(Message::Operation::REQUEST, buffer, size_written);

			LOG_DEBUG("Waiting for task...");
			LOG_DEBUG("Got: %d", *(DCS::u16*)task.get().ptr);


			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			Client::StopThread(c);
		}
	});
	
	nt.join();

	Destroy();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
