#include "../include/internal.h"

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

	static GUID GUID_DEVINTERFACE_USB_HUB             = { 0xf18a0e88, 0xc30c, 0x11d0, { 0x88, 0x15, 0x00, 0xa0, 0xc9, 0x06, 0xbe, 0xd8 } };
	static GUID GUID_DEVINTERFACE_USB_DEVICE          = { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
	static GUID GUID_DEVINTERFACE_USB_HOST_CONTROLLER = { 0x3abf6f2d, 0x71c4, 0x462a, { 0x8a, 0x92, 0x1e, 0x68, 0x61, 0xe6, 0xaf, 0x27 } };
	static GUID GUID_DEVINTERFACE_USB_GENERIC         = { 0x4d36e96e, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

	// Get usb device interfaces
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

	// Returns last found value
	// VID - PID is pretty specific, so nothing will come to harm
	return DevicePath;
}


static BOOL QueryDeviceEndpoints(WINUSB_INTERFACE_HANDLE hDeviceHandle, DCS::USerial::PIPE_ID* pipeid)
{
	if (hDeviceHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	BOOL bResult = TRUE;

	USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
	ZeroMemory(&InterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

	WINUSB_PIPE_INFORMATION  Pipe;
	ZeroMemory(&Pipe, sizeof(WINUSB_PIPE_INFORMATION));

	bResult = WinUsb_QueryInterfaceSettings(hDeviceHandle, 0, &InterfaceDescriptor);

	if (bResult)
	{
		for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
		{
			bResult = WinUsb_QueryPipe(hDeviceHandle, 0, index, &Pipe);

			if (bResult)
			{
				// Only care about the UsbdPipeTypeBulk endpoints
				if (Pipe.PipeType == UsbdPipeTypeBulk)
				{
					if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
					{
						LOG_DEBUG("Got IN Endpoint index: %d Pipe type: %d Bulk Pipe ID: %d.\n", index, Pipe.PipeType, Pipe.PipeId);
						pipeid->PipeInId = Pipe.PipeId;
					}
					if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
					{
						LOG_DEBUG("Got OUT Endpoint index: %d Pipe type: %d Bulk Pipe ID: %d.\n", index, Pipe.PipeType, Pipe.PipeId);
						pipeid->PipeOutId = Pipe.PipeId;
					}
				}
			}
			else
			{
				continue;
			}
		}
	}

	return bResult;
}

static ULONG WriteToBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, UCHAR* buffer, ULONG size)
{
	if (hDeviceHandle == INVALID_HANDLE_VALUE)
	{
		LOG_ERROR("Attempting to Write to usb endpoint using an invalid WinUSB handle. Aborting...");
		return 0;
	}

	ULONG cbSent = 0;
	WinUsb_WritePipe(hDeviceHandle, pID, buffer, size, &cbSent, 0);

	return cbSent;
}

static ULONG ReadFromBulkEndpoint(WINUSB_INTERFACE_HANDLE hDeviceHandle, UCHAR pID, UCHAR* buffer, ULONG size)
{
	if (hDeviceHandle == INVALID_HANDLE_VALUE)
	{
		LOG_ERROR("Attempting to Read from usb endpoint using an invalid WinUSB handle. Aborting...");
		return 0;
	}

	ULONG cbRead = 0;
	WinUsb_ReadPipe(hDeviceHandle, pID, buffer, size, &cbRead, 0);

	return cbRead;
}

ULONG DCS::USerial::write_bulk_bytes(USBIntHandle hnd, PUCHAR buffer, DWORD size)
{
	//LOG_DEBUG("Writting...");
	return WriteToBulkEndpoint(hnd.usb_handle, hnd.pipe_id.PipeOutId, buffer, size);
}

ULONG DCS::USerial::read_bulk_bytes(USBIntHandle hnd, PUCHAR buffer, DWORD size)
{
	ULONG sz = ReadFromBulkEndpoint(hnd.usb_handle, hnd.pipe_id.PipeInId, buffer, size);
	buffer[sz - 1] = '\0';

	return sz;
}

DCS::USerial::USBIntHandle DCS::USerial::init_usb_handle(std::string VID_PID)
{
	LPTSTR devPath = GetDeviceUSBPath(VID_PID.substr(0, 4), VID_PID.substr(5, 4));
	LOG_DEBUG("Initializing USB - DevicePath: %s", devPath);

	HANDLE DeviceHandle = CreateFile(devPath,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	if (INVALID_HANDLE_VALUE == DeviceHandle) {

		LOG_ERROR("Failed to open device path: %s", devPath);
		return { NULL, INVALID_HANDLE_VALUE, PIPE_ID() };
	}

	GlobalFree(devPath);

	WINUSB_INTERFACE_HANDLE WinusbHandle = INVALID_HANDLE_VALUE;

	BOOL bResult = WinUsb_Initialize(DeviceHandle, &WinusbHandle);

	if (FALSE == bResult) {

		LOG_ERROR("Failed to initialize WinUSB. Error: %ld", GetLastError());
		CloseHandle(DeviceHandle);
		return { INVALID_HANDLE_VALUE, NULL, PIPE_ID() };
	}
	else
	{
		PIPE_ID PipeID;
		BOOL bQResult = QueryDeviceEndpoints(WinusbHandle, &PipeID);

		if (FALSE == bQResult)
		{
			LOG_ERROR("Failed to query usb endpoints. Error: %ld", GetLastError());
			CloseHandle(DeviceHandle);
			return { NULL, NULL, PIPE_ID() };
		}
		else
		{
			LOG_DEBUG("WinUSB initialized successfully.");
			return { WinusbHandle, DeviceHandle, PipeID };
		}
	}
}

BOOL DCS::USerial::term_usb_handle(USBIntHandle handle)
{
	BOOL v = WinUsb_Free(handle.usb_handle);
	v &= CloseHandle(handle.dev_handle);
	return v;
}