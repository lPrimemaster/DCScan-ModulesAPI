#include <iostream>
#include <DCS_Core/include/DCS_ModuleCore.h>
#include <DCS_Utils/include/DCS_ModuleUtils.h>
#include <DCS_Network/include/DCS_ModuleNetwork.h>

int main()
{
	using namespace DCS::Network; // For Server and Socket
	using namespace DCS::Utils;   // For Logger
	Socket s = Server::Create(15777);

	// This waits for a client to connect (blocking)
	Socket client = Server::WaitForConnection(s);
	Server::StartThread(client, [](const char* data, DCS::i64 size, Socket client)->void {	
		Logger::Debug("Received data: %s", data);	
		Server::SendData(client, data, size);		
		});
	// Will run indefinitely until client disconnect, or...
	std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait one second to issue stop command
	Server::StopThread(client);
	return 0;
}
