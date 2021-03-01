#include <iostream>
#include <DCS_Core/include/DCS_ModuleCore.h>
#include <DCS_Utils/include/DCS_ModuleUtils.h>
#include <DCS_Network/include/DCS_ModuleNetwork.h>

int main()
{
	using namespace DCS::Network; // For Client and Socket

	Socket c = Client::Connect("127.0.0.1", 15777);

	Client::StartThread(c, [](const char* data, DCS::i32 size, Socket client)->void {
		LOG_DEBUG("Received data from server: %s", data);
		Client::SendData(client, data, size);
		});
	// Will run indefinitely until server disconnect, or...
	std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait one second to issue stop command
	Client::StopThread(client);
	return 0;
}
