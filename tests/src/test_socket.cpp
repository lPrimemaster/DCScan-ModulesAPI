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
	using namespace DCS::Utils;

	Logger::Message("This test requires python3.");

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
	s.sendall(b'testing data flow')
	time.sleep(0.1)
	s.sendall(b'or maybe... mooore data???')
	time.sleep(0.1)
	s.sendall(b'D\0')
	time.sleep(0.1)
	s.sendall(b'this has to hash a similar thing...')
		)";

		FILE* f = fopen("Release/test_socket_endpoint.py", "w");
		if (f)
		{
			fwrite(code, 1, strlen(code), f);
			fclose(f);
		}

		std::this_thread::sleep_for(std::chrono::seconds(2));

		while (ready.load() == 0)
			std::this_thread::yield();

		system("python Release/test_socket_endpoint.py");
	});

	ready.store(1);

	ServerListen(server);
	SOCKET client = INVALID_SOCKET;
	client = ServerAcceptConnection(server);

	char buffer[256] = {0};
	size_t recv_sz;

	do
	{
		recv_sz = ServerReceiveData(client, buffer, 256);
		Logger::Debug("Bytes[%d]\tData[%s]", recv_sz, buffer);
	} while (recv_sz > 0);

	SOCKET sockets[] = { server, client };

	CleanupWinSock(sockets, 2);

	nt.join();

	std::hash<std::string> hasher;

	DCS_ASSERT_EQ(hasher(buffer), hasher("this has to hash a similar thing..."));

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}