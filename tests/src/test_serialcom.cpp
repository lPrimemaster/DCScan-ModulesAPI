#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_EngineControl/include/internal.h"
#include "../include/DCS_Assert.h"

// std::to_string
#include <string>

bool issueCommand(HANDLE handle, const std::string command, const int axis, const std::string right)
{
	const std::string value = axis ? std::to_string(axis) + command + right + ';' : command + right + ';';
	const char* data = value.c_str();

	bool stt = true;

	stt &= DCS::Serial::write_bytes(handle, data, value.size());
	stt &= DCS::Serial::write_bytes(handle, "\r", 2);

	return stt;
}

int main()
{
	DCS_START_TEST;

	using namespace DCS::Serial;

	SerialArgs args;
	args.baudRate = 921600;		//921.6 kBd
	args.byteSize = 8;			//8 bit size
	args.eofChar = '\r';		//Carriage return command eof (?)
	args.parity = NOPARITY;		//No parity
	args.stopBits = ONESTOPBIT;	//One stop bit

	HANDLE handle = init_handle("COM3", GENERIC_READ | GENERIC_WRITE, args);

	issueCommand(handle, "MO", 2, "");

	/*issueCommand(handle, "PA", 2, "45.000");
	issueCommand(handle, "WS", 2, "");
	issueCommand(handle, "TP", 2, "?");*/

	issueCommand(handle, "OR", 2, "");
	issueCommand(handle, "WS", 2, "");
	issueCommand(handle, "TP", 2, "?");

	

	char response[256];
	DWORD rbSize = 0;
	bool val = read_bytes(handle, response, 256, &rbSize);

	LOG_DEBUG("2TP? Got: %s", response);

	issueCommand(handle, "MF", 2, "");


	close_handle(handle);

	DCS_RETURN_TEST;
}