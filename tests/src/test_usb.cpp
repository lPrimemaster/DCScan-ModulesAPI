#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_EngineControl/include/internal.h"

#include "../include/DCS_Assert.h"


int main(int argc, char* argv[])
{
	DCS_START_TEST;

	DCS::USerial::USBIntHandle hnd = DCS::USerial::init_usb_handle("104D-4000");

	auto v = ("2>1PA" + std::string(argv[1]) + "\r");

	DCS::USerial::write_bulk_bytes(hnd, (PUCHAR)v.c_str(), v.size());

	std::this_thread::sleep_for(std::chrono::seconds(3));

	DCS::USerial::write_bulk_bytes(hnd, (PUCHAR)"2>1TP?\r", 7);

	char buffer[256];

	DCS::USerial::read_bulk_bytes(hnd, (PUCHAR)buffer, 256);

	LOG_DEBUG("Got response: %s", buffer);

	DCS::USerial::write_bulk_bytes(hnd, (PUCHAR)"2>1PA0", 6);

	LOG_DEBUG("Waiting 2 seconds to reset axis 1...");
	std::this_thread::sleep_for(std::chrono::seconds(2));

	DCS::USerial::write_bulk_bytes(hnd, (PUCHAR)"\r", 1);

	std::this_thread::sleep_for(std::chrono::seconds(2));

	DCS::USerial::term_usb_handle(hnd);

	DCS_RETURN_TEST;
}