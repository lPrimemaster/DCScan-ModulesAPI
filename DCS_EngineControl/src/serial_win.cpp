#include "../include/DCS_ModuleEngineControl.h"
#include "../include/internal.h"

#include <stdio.h>

#define SERIAL_DEFAULT_BAUD 921600
#define SERIAL_DEFAULT_BYTE 8
#define SERIAL_DEFAULT_SBIT ONESTOPBIT
#define SERIAL_DEFAULT_PARY NOPARITY
#define SERIAL_DEFAULT_EOFC '\r'

#define SERIAL_DEFAULT_READ_INTERVAL_TIMEOUT 50
#define SERIAL_DEFAULT_READ_TOTAL_TIMEOUT_CT 50
#define SERIAL_DEFAULT_READ_TOTAL_MULTIPLIER 10
#define SERIAL_DEFAULT_WRITE_TOTAL_TIMEOUT_CT 50
#define SERIAL_DEFAULT_WRITE_TOTAL_MULTIPLIER 10

#define SERIAL_DEFAULT_READ_BUFFER_SIZE 256
#define SERIAL_DEFAULT_READ_RX_EOF '\n'

HANDLE DCS::Serial::init_handle(LPCSTR portName, DWORD rwAccess, SerialArgs args)
{
	//HANDLE of the COMport device [serial init]
	HANDLE hComm = CreateFile(portName, rwAccess, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hComm == INVALID_HANDLE_VALUE)
	{
		LOG_CRITICAL("Serial: Port %s error. Not opened.", portName);
		LOG_CRITICAL("Serial: Extended error: %u", GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	else
	{
		LOG_DEBUG("Serial: Port %s opened.", portName);
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	BOOL status = GetCommState(hComm, &dcbSerialParams); //Get the serial parameters

	if (status == FALSE)
		LOG_WARNING("Serial: GetCommState() error. Using default or suplied values instead.");

	/* Defaults - BDR: 921.6kBd - BSZ: 8bits - SBT: OneStop - PAR: NoParity - EOFC: Carriage return */
	dcbSerialParams.BaudRate = args.baudRate ? args.baudRate : (dcbSerialParams.BaudRate ? dcbSerialParams.BaudRate : SERIAL_DEFAULT_BAUD);
	dcbSerialParams.ByteSize = args.byteSize ? args.byteSize : (dcbSerialParams.ByteSize ? dcbSerialParams.ByteSize : SERIAL_DEFAULT_BYTE);
	dcbSerialParams.StopBits = args.stopBits ? args.stopBits : (dcbSerialParams.StopBits ? dcbSerialParams.StopBits : SERIAL_DEFAULT_SBIT);
	dcbSerialParams.Parity = args.parity ? args.parity : (dcbSerialParams.Parity ? dcbSerialParams.Parity : SERIAL_DEFAULT_PARY);
	//dcbSerialParams.EofChar  = args.eofChar  ? args.eofChar  : (dcbSerialParams.EofChar  ? dcbSerialParams.EofChar  : SERIAL_DEFAULT_EOFC);

	status = SetCommState(hComm, &dcbSerialParams); //[Re]configure the port with DCB params

	if (status == FALSE)
	{
		LOG_WARNING("Serial: SetCommState() error. Using default values, if applicable.");
		LOG_WARNING("Serial: Set COM DCB Structure Fail. Using intrinsic values, if applicable.");
		LOG_WARNING("       Baudrate = %d", dcbSerialParams.BaudRate);
		LOG_WARNING("		ByteSize = %d", dcbSerialParams.ByteSize);
		LOG_WARNING("		StopBits = %d", dcbSerialParams.StopBits);
		LOG_WARNING("		Parity   = %d", dcbSerialParams.Parity);
		LOG_WARNING("		EOFChar  = %d", dcbSerialParams.EofChar);
	}
	else
	{
		LOG_DEBUG("Serial: Set COM DCB Structure Success.");
		LOG_DEBUG("		Baudrate = %d", dcbSerialParams.BaudRate);
		LOG_DEBUG("		ByteSize = %d", dcbSerialParams.ByteSize);
		LOG_DEBUG("		StopBits = %d", dcbSerialParams.StopBits);
		LOG_DEBUG("		Parity   = %d", dcbSerialParams.Parity);
		LOG_DEBUG("		EOFChar  = %d", dcbSerialParams.EofChar);
	}

	COMMTIMEOUTS timeouts = { 0 };

	timeouts.ReadIntervalTimeout = args.readIntervalTimeout ? args.readIntervalTimeout : SERIAL_DEFAULT_READ_INTERVAL_TIMEOUT;
	timeouts.ReadTotalTimeoutConstant = args.readTotalTimeoutConstant ? args.readTotalTimeoutConstant : SERIAL_DEFAULT_READ_TOTAL_TIMEOUT_CT;
	timeouts.ReadTotalTimeoutMultiplier = args.readTotalTimeoutMultiplier ? args.readTotalTimeoutMultiplier : SERIAL_DEFAULT_READ_TOTAL_MULTIPLIER;
	timeouts.WriteTotalTimeoutConstant = args.writeTotalTimeoutConstant ? args.writeTotalTimeoutConstant : SERIAL_DEFAULT_WRITE_TOTAL_TIMEOUT_CT;
	timeouts.WriteTotalTimeoutMultiplier = args.writeTotalTimeoutMultiplier ? args.writeTotalTimeoutMultiplier : SERIAL_DEFAULT_WRITE_TOTAL_MULTIPLIER;

	//Use this value for checking - but all must be initialized
	if (timeouts.ReadIntervalTimeout)
	{
		status = SetCommTimeouts(hComm, &timeouts);
		if (status == FALSE)
		{
			LOG_ERROR("Serial: SetCommTimeouts() error. Using no values for timeouts.");
		}
		else
		{
			LOG_DEBUG("Serial: Set port timeouts successfull.");
		}
	}

	LOG_DEBUG("Serial: Connection initialized successfully.");

	return hComm;
}

BOOL DCS::Serial::write_bytes(HANDLE hComm, LPCSTR charArray, DWORD NbytesToWrite)
{
	//Check if the handle is in fact valid
	if (hComm == INVALID_HANDLE_VALUE)
	{
		LOG_CRITICAL("Serial: HANDLE %u is invalid.\n", (uintptr_t)hComm);
		return FALSE;
	}

	DWORD NbytesWritten = 0;

	//Tx the array to the device
	BOOL status = WriteFile(hComm, charArray, NbytesToWrite, &NbytesWritten, NULL);

	if (status == FALSE)
	{
		LOG_CRITICAL("Serial: Tx of data failed.");
		LOG_CRITICAL("		Serial port = %u", (uintptr_t)hComm);
		LOG_CRITICAL("		Serial data = %s", charArray);
		LOG_CRITICAL("		Error       = %u", GetLastError());
		return FALSE;
	}

	LOG_DEBUG("Serial: Tx of data succeded. [%ull bytes written]", NbytesWritten);

	return TRUE;

}

BOOL DCS::Serial::read_bytes(HANDLE hComm, LPTSTR buffer, DWORD bufferSize, LPDWORD readBufferSize)
{
	//Check if the handle is in fact valid
	if (hComm == INVALID_HANDLE_VALUE)
	{
		LOG_CRITICAL("Serial: HANDLE %u is invalid.", (uintptr_t)hComm);
		return FALSE;
	}

	DWORD dwEventMask = 0;
	char  localSerialBuffer[SERIAL_DEFAULT_READ_BUFFER_SIZE];

	//Receive any byte
	BOOL status = SetCommMask(hComm, EV_RXCHAR);

	if (status == FALSE)
	{
		LOG_CRITICAL("Serial: SetCommMask() error. Event %d might be invalid.", EV_RXCHAR);
		LOG_CRITICAL("Serial: Extended error: %u", GetLastError());
		return FALSE;
	}

	//Wait for the data
	status = WaitCommEvent(hComm, &dwEventMask, NULL);

	if (status == FALSE)
	{
		LOG_CRITICAL("Serial: WaitCommEvent() error.");
		LOG_CRITICAL("Serial: Extended error: %u", GetLastError());
		return FALSE;
	}

	//Start reading the Rxed data
	int i = 0;
	int excess = 0;
	int last = 0;
	while (TRUE)
	{
		char tempChar = 0;
		DWORD NbytesRead = 0;

		status = ReadFile(hComm, &tempChar, sizeof(tempChar), &NbytesRead, NULL);
		if (status == FALSE) break;

		if (i == SERIAL_DEFAULT_READ_BUFFER_SIZE)
		{
			excess++;
			if (tempChar == SERIAL_DEFAULT_READ_RX_EOF) break;
			continue;
		}

		last = i - 1;
		localSerialBuffer[i++] = tempChar;

		if (tempChar == SERIAL_DEFAULT_READ_RX_EOF) break;
	}

	if (i == SERIAL_DEFAULT_READ_BUFFER_SIZE && localSerialBuffer[SERIAL_DEFAULT_READ_BUFFER_SIZE - 1] != SERIAL_DEFAULT_READ_RX_EOF)
	{
		LOG_ERROR("Serial: Rx of data incomplete.");
		LOG_ERROR("		Read Buffer Size = %d", SERIAL_DEFAULT_READ_BUFFER_SIZE);
		LOG_ERROR("		Received Size    = %d", excess + i);
		LOG_ERROR("		Ignoring Size    = %d", excess);
		LOG_ERROR("		Received Bytes   = %s", localSerialBuffer);
		LOG_ERROR("		Terminated       = %s", "False");
		return FALSE;
	}

	if (status == FALSE)
	{
		LOG_ERROR("Serial: Rx of data failed.");
		LOG_ERROR("Serial: Extended error: %u", GetLastError());
		LOG_ERROR("		Read Buffer Size = %d", SERIAL_DEFAULT_READ_BUFFER_SIZE);
		LOG_ERROR("		Received Size    = %d", excess + i);
		LOG_ERROR("		Ignoring Size    = %d", excess);
		LOG_ERROR("		Received Bytes   = %s", localSerialBuffer);
		LOG_ERROR("		Terminated       = %s", localSerialBuffer[last] == '\n' ? "True" : "False");
		return FALSE;
	}

	//Replace the terminator char for a string terminator -> last = \r
	localSerialBuffer[last] = '\0';

	//Copy the data to the argument buffer
	DWORD localBufferSize = (DWORD)strlen(localSerialBuffer);

	if (bufferSize >= localBufferSize)
	{
		strcpy_s(buffer, bufferSize, localSerialBuffer);
		*readBufferSize = localBufferSize;
	}
	else
	{
		LOG_ERROR("Serial: Rx copy of data failed.\n");
		LOG_ERROR("Serial: Size of user buffer is too small.\n");
		LOG_ERROR("		Read Buffer Size = %d\n", SERIAL_DEFAULT_READ_BUFFER_SIZE);
		LOG_ERROR("		User Buffer Size = %d\n", bufferSize);
		return FALSE;
	}

	LOG_DEBUG("Serial: Rx of data succeded. [%d bytes read]", last);

	return TRUE;
}

BOOL DCS::Serial::close_handle(HANDLE hComm)
{
	LOG_DEBUG("Serial: Closing handle %x", hComm);
	return CloseHandle(hComm);
}
