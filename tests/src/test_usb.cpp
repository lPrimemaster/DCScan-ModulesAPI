#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_EngineControl/include/internal.h"

#include "../include/DCS_Assert.h"


int main()
{
	DCS_START_TEST;

	DCS::USerial::init_usb_handle();

	DCS_RETURN_TEST;
}