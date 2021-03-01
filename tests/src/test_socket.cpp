#include "../../DCS_Network/include/DCS_ModuleNetwork.h"
#include "../../DCS_Network/include/internal.h"
#include "../include/DCS_Assert.h"

#include <string>
#include <unordered_map>
#include <atomic>


int testInternal()
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

std::string fdata;
std::mutex m;
std::atomic<int> ready = 0;

int test()
{
	DCS_START_TEST;

	using namespace DCS::Network;

	

	Socket s = Server::Create(15777);

	std::thread nt([&]()-> void {
		const char* code = R"(
import socket
import time

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 15777        # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.connect((HOST, PORT))
	s.sendall(b'\x0C\x00\x00\x00') # 12 bytes incoming
	s.sendall(b'abc')
	s.sendall(b'def')
	time.sleep(0.5)
	s.sendall(b'ghi')
	s.sendall(b'jkl')
		)";

		FILE* f = fopen("Release/test_socket_endpoint2.py", "w");
		if (f)
		{
			fwrite(code, 1, strlen(code), f);
			fclose(f);
		}

		while (ready.load() == 0)
			std::this_thread::yield();

		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		system("python Release/test_socket_endpoint2.py");
		});

	ready.store(1);
	Socket client = Server::WaitForConnection(s);

	Server::StartThread(client, [](const unsigned char* data, DCS::i32 size, Socket client)->void {
		LOG_DEBUG("Received data: %s", (char*)data);
		fdata = std::string((char*)data, size);
		ready.store(0);
		});

	while (ready.load() == 1)
		std::this_thread::yield();

	Server::StopThread(client);

	nt.join();

	std::hash<std::string> hasher;

	DCS_ASSERT_EQ(hasher(fdata), hasher("abcdefghijkl"));

	DCS_RETURN_TEST;
}

int main()
{
	return testInternal() || test();
}