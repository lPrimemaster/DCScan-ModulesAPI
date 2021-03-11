#include <iostream>
#include <DCS_Core/include/DCS_ModuleCore.h>
#include <DCS_Utils/include/DCS_ModuleUtils.h>
#include <DCS_Network/include/DCS_ModuleNetwork.h>

int main()
{
	using namespace DCS::Network; // For Client and Socket

	Socket c = Client::Connect("127.0.0.1", 15777);

	Client::StartThread(c);

	// Create a message to execute a server call
	unsigned char buffer[512];
	auto size = DCS::Registry::SVParams::GetDataFromParams(buffer,
		SV_CALL_DCS_Threading_addInt,
		42, 1);

	// Send data to execute function in server and waits for response
	auto sum_r = Message::SendSync(Message::Operation::REQUEST, buffer, size);

	std::cout << "42 + 1 = " << *(int*)sum_r.ptr << std::endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait one second to issue stop command
	Client::StopThread(client);
	return 0;
}
