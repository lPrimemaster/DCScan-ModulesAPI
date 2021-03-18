#include "../include/internal.h"

#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>
#include <strsafe.h>
#include <cfgmgr32.h>
#include <tchar.h>
#include <devpkey.h>

typedef struct _DEVICE_DATA {
	BOOL HandlesOpen;
	WINUSB_INTERFACE_HANDLE WinusbHandle;
	HANDLE DeviceHandle;
	LPTSTR DevicePath;
} DEVICE_DATA, * PDEVICE_DATA;

// Skipping error checking is not very healthy...
static LPTSTR GetDeviceUSBPath(std::string vid, std::string pid)
{
	DWORD deviceIndex = 0;
	SP_DEVINFO_DATA deviceInfoData;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

	deviceInfoData.cbSize = sizeof(deviceInfoData);


	static GUID GUID_DEVINTERFACE_USB_HUB = { 0xf18a0e88, 0xc30c, 0x11d0, {0x88, 0x15, 0x00, 0xa0, 0xc9, 0x06, 0xbe, 0xd8} };
	static GUID GUID_DEVINTERFACE_USB_DEVICE = { 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
	static GUID GUID_DEVINTERFACE_USB_HOST_CONTROLLER = { 0x3abf6f2d, 0x71c4, 0x462a, {0x8a, 0x92, 0x1e, 0x68, 0x61, 0xe6, 0xaf, 0x27} };
	static GUID GUID_DEVINTERFACE_USB_GENERIC = { 0x4d36e96e, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

	//get usb device interfaces
	HDEVINFO deviceInterfaceSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_DEVICEINTERFACE);

	LPTSTR DevicePath = (LPTSTR)GlobalAlloc(GMEM_FIXED, 256);

	while (SetupDiEnumDeviceInfo(deviceInterfaceSet, deviceIndex, &deviceInfoData))
	{
		deviceInfoData.cbSize = sizeof(deviceInfoData);

		ULONG IDSize;
		CM_Get_Device_ID_Size(&IDSize, deviceInfoData.DevInst, 0);

		TCHAR* deviceID = new TCHAR[IDSize];

		CM_Get_Device_ID(deviceInfoData.DevInst, deviceID, MAX_PATH, 0);

		if (deviceID[8] == vid.at(0) && deviceID[9] == vid.at(1) && deviceID[10] == vid.at(2) && deviceID[11] == vid.at(3) && //VID
			deviceID[17] == pid.at(0) && deviceID[18] == pid.at(1) && deviceID[19] == pid.at(2) && deviceID[20] == pid.at(3)) //PID
		{
			DWORD deviceInterfaceIndex = 0;
			deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

			while (SetupDiEnumDeviceInterfaces(deviceInterfaceSet, &deviceInfoData, &GUID_DEVINTERFACE_USB_DEVICE, deviceInterfaceIndex, &deviceInterfaceData))
			{
				deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

				// Get some more details etc
				DWORD requiredBufferSize;
				SetupDiGetDeviceInterfaceDetail(deviceInterfaceSet,
					&deviceInterfaceData,
					NULL,
					0,
					&requiredBufferSize,
					NULL);

				PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED, requiredBufferSize);
				detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				ULONG length = requiredBufferSize;

				SetupDiGetDeviceInterfaceDetail(deviceInterfaceSet,
					&deviceInterfaceData,
					detailData,
					length,
					&requiredBufferSize,
					NULL);

				
				StringCbCopy(DevicePath, 256, detailData->DevicePath);

				LocalFree(detailData);
				deviceInterfaceIndex++;
			}
		}

		deviceIndex++;
	}

	// Returns last value
	return DevicePath;
}

HANDLE DCS::USerial::init_usb_handle()
{
	DEVICE_DATA devData;
	BOOL onFailure;

	//HRESULT v = OpenDevice(&devData, &onFailure);
	LPTSTR devPath = GetDeviceUSBPath("256C", "006D");
	LOG_DEBUG("Device Path: %s", devPath);
	GlobalFree(devPath);

	HANDLE DeviceHandle = CreateFile(devPath,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	if (INVALID_HANDLE_VALUE == DeviceHandle) {

		LOG_DEBUG("Failed to open...");
	}

	WINUSB_INTERFACE_HANDLE WinusbHandle;

	BOOL bResult = WinUsb_Initialize(DeviceHandle, &WinusbHandle);

	if (FALSE == bResult) {

		LOG_DEBUG("%ld", GetLastError());
		CloseHandle(DeviceHandle);
	}
	else
	{
		WinUsb_Free(WinusbHandle);
		CloseHandle(DeviceHandle);
	}



	//LOG_DEBUG("Open device return code: %ld", v);
	return INVALID_HANDLE_VALUE;
}