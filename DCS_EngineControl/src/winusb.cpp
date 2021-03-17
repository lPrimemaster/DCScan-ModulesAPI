#include "../include/internal.h"

#include <winusb.h>
#include <Usb100.h>
#include <Setupapi.h>
#include <strsafe.h>

typedef struct _DEVICE_DATA {
	BOOL HandlesOpen;
	WINUSB_INTERFACE_HANDLE WinusbHandle;
	HANDLE DeviceHandle;
	LPTSTR DevicePath;
} DEVICE_DATA, * PDEVICE_DATA;

static
HRESULT
RetrieveDevicePath(
	_Out_bytecap_(BufLen) LPTSTR DevicePath,
	_In_                  ULONG  BufLen,
	_Out_opt_             PBOOL  FailureDeviceNotFound
)
{
	BOOL                             bResult = FALSE;
	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         interfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
	ULONG                            length;
	ULONG                            requiredLength = 0;
	HRESULT                          hr;

	if (NULL != FailureDeviceNotFound) {

		*FailureDeviceNotFound = FALSE;
	}

	//
	// Enumerate all devices exposing the interface
	//
	deviceInfo = SetupDiGetClassDevs(&DCS::USerial::OSR_DEVICE_INTERFACE,
		NULL,
		NULL,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (deviceInfo == INVALID_HANDLE_VALUE) {

		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	//
	// Get the first interface (index 0) in the result set
	//
	bResult = SetupDiEnumDeviceInterfaces(deviceInfo,
		NULL,
		&DCS::USerial::OSR_DEVICE_INTERFACE,
		0,
		&interfaceData);

	if (FALSE == bResult) {

		//
		// We would see this error if no devices were found
		//
		if (ERROR_NO_MORE_ITEMS == GetLastError() &&
			NULL != FailureDeviceNotFound) {

			*FailureDeviceNotFound = TRUE;
		}

		hr = HRESULT_FROM_WIN32(GetLastError());
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return hr;
	}

	//
	// Get the size of the path string
	// We expect to get a failure with insufficient buffer
	//
	bResult = SetupDiGetDeviceInterfaceDetail(deviceInfo,
		&interfaceData,
		NULL,
		0,
		&requiredLength,
		NULL);

	if (FALSE == bResult && ERROR_INSUFFICIENT_BUFFER != GetLastError()) {

		hr = HRESULT_FROM_WIN32(GetLastError());
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return hr;
	}

	//
	// Allocate temporary space for SetupDi structure
	//
	detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
		LocalAlloc(LMEM_FIXED, requiredLength);

	if (NULL == detailData)
	{
		hr = E_OUTOFMEMORY;
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return hr;
	}

	detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	length = requiredLength;

	//
	// Get the interface's path string
	//
	bResult = SetupDiGetDeviceInterfaceDetail(deviceInfo,
		&interfaceData,
		detailData,
		length,
		&requiredLength,
		NULL);

	if (FALSE == bResult)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LocalFree(detailData);
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return hr;
	}

	//
	// Give path to the caller. SetupDiGetDeviceInterfaceDetail ensured
	// DevicePath is NULL-terminated.
	//
	hr = StringCbCopy(DevicePath,
		BufLen,
		detailData->DevicePath);

	LocalFree(detailData);
	SetupDiDestroyDeviceInfoList(deviceInfo);

	return hr;
}

static
HRESULT
OpenDevice(
	_Out_     PDEVICE_DATA DeviceData,
	_Out_opt_ PBOOL        FailureDeviceNotFound
)
{
	HRESULT hr = S_OK;
	BOOL    bResult;

	DeviceData->HandlesOpen = FALSE;

	hr = RetrieveDevicePath(DeviceData->DevicePath,
		sizeof(DeviceData->DevicePath),
		FailureDeviceNotFound);

	if (FAILED(hr)) {

		return hr;
	}

	DeviceData->DeviceHandle = CreateFile(DeviceData->DevicePath,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	if (INVALID_HANDLE_VALUE == DeviceData->DeviceHandle) {

		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	bResult = WinUsb_Initialize(DeviceData->DeviceHandle,
		&DeviceData->WinusbHandle);

	if (FALSE == bResult) {

		hr = HRESULT_FROM_WIN32(GetLastError());
		CloseHandle(DeviceData->DeviceHandle);
		return hr;
	}

	DeviceData->HandlesOpen = TRUE;
	return hr;
}

HANDLE DCS::USerial::init_usb_handle()
{
	DEVICE_DATA devData;
	BOOL onFailure;

	HRESULT v = OpenDevice(&devData, &onFailure);

	LOG_DEBUG("Open device return code: %d", v);
	return INVALID_HANDLE_VALUE;
}