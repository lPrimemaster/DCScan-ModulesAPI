#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_EngineControl/include/internal.h"

#include "../include/DCS_Assert.h"


int main()
{
	DCS_START_TEST;

	HANDLE h = DCS::USerial::init_usb_handle();

	if (h == INVALID_HANDLE_VALUE)
	{
		LOG_DEBUG("OOPS");
	}

	DCS_RETURN_TEST;
}