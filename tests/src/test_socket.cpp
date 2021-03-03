#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Network/include/internal.h"
#include "../include/DCS_Assert.h"

#include <string>
#include <unordered_map>
#include <atomic>


int test()
{
	DCS_START_TEST;

	using namespace DCS::Network;

	LOG_MESSAGE("This test requires python3.");

	std::atomic<int> ready = 0;

	InitWinSock();

	SOCKET server = CreateServerSocket(15777);

	std::thread nt([&]()-> void {
		const char* code = R"(
import socket
import time

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 15777        # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.connect((HOST, PORT))
	s.sendall(b'this has to hash the same thing...\0')
		)";

		FILE* f = fopen("Release/test_socket_endpoint.py", "w");
		if (f)
		{
			fwrite(code, 1, strlen(code), f);
			fclose(f);
		}

		while (ready.load() == 0)
			std::this_thread::yield();

		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		system("python Release/test_socket_endpoint.py");
	});

	ready.store(1);

	ServerListen(server);
	SOCKET client = INVALID_SOCKET;
	client = ServerAcceptConnection(server);

	unsigned char buffer[256] = {0};
	size_t recv_sz;

	do
	{
		recv_sz = ReceiveData(client, buffer, 256);
		LOG_DEBUG("Bytes[%d]\tData[%s]", recv_sz, (char*)buffer);
	} while (recv_sz > 0);

	CloseSocketConnection(client);

	CleanupWinSock();

	nt.join();

	std::hash<std::string> hasher;

	DCS_ASSERT_EQ(hasher((char*)buffer), hasher("this has to hash the same thing...\0"));

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}