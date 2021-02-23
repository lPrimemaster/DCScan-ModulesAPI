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
		DCS::Utils::Logger::Critical("Serial: Port %s error. Not opened.", portName);
		DCS::Utils::Logger::Critical("Serial: Extended error: %u", GetLastError());
		return INVALID_HANDLE_VALUE;
	}
	else
	{
		DCS::Utils::Logger::Debug("Serial: Port %s opened.", portName);
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	BOOL status = GetCommState(hComm, &dcbSerialParams); //Get the serial parameters

	if (status == FALSE)
		DCS::Utils::Logger::Warning("Serial: GetCommState() error. Using default or suplied values instead.");

	/* Defaults - BDR: 921.6kBd - BSZ: 8bits - SBT: OneStop - PAR: NoParity - EOFC: Carriage return */
	dcbSerialParams.BaudRate = args.baudRate ? args.baudRate : (dcbSerialParams.BaudRate ? dcbSerialParams.BaudRate : SERIAL_DEFAULT_BAUD);
	dcbSerialParams.ByteSize = args.byteSize ? args.byteSize : (dcbSerialParams.ByteSize ? dcbSerialParams.ByteSize : SERIAL_DEFAULT_BYTE);
	dcbSerialParams.StopBits = args.stopBits ? args.stopBits : (dcbSerialParams.StopBits ? dcbSerialParams.StopBits : SERIAL_DEFAULT_SBIT);
	dcbSerialParams.Parity = args.parity ? args.parity : (dcbSerialParams.Parity ? dcbSerialParams.Parity : SERIAL_DEFAULT_PARY);
	//dcbSerialParams.EofChar  = args.eofChar  ? args.eofChar  : (dcbSerialParams.EofChar  ? dcbSerialParams.EofChar  : SERIAL_DEFAULT_EOFC);

	status = SetCommState(hComm, &dcbSerialParams); //[Re]configure the port with DCB params

	if (status == FALSE)
	{
		DCS::Utils::Logger::Warning("Serial: SetCommState() error. Using default values, if applicable.");
		DCS::Utils::Logger::Warning("Serial: Set COM DCB Structure Fail. Using intrinsic values, if applicable.");
		DCS::Utils::Logger::Warning("       Baudrate = %d", dcbSerialParams.BaudRate);
		DCS::Utils::Logger::Warning("		ByteSize = %d", dcbSerialParams.ByteSize);
		DCS::Utils::Logger::Warning("		StopBits = %d", dcbSerialParams.StopBits);
		DCS::Utils::Logger::Warning("		Parity   = %d", dcbSerialParams.Parity);
		DCS::Utils::Logger::Warning("		EOFChar  = %d", dcbSerialParams.EofChar);
	}
	else
	{
		DCS::Utils::Logger::Debug("Serial: Set COM DCB Structure Success.");
		DCS::Utils::Logger::Debug("		Baudrate = %d", dcbSerialParams.BaudRate);
		DCS::Utils::Logger::Debug("		ByteSize = %d", dcbSerialParams.ByteSize);
		DCS::Utils::Logger::Debug("		StopBits = %d", dcbSerialParams.StopBits);
		DCS::Utils::Logger::Debug("		Parity   = %d", dcbSerialParams.Parity);
		DCS::Utils::Logger::Debug("		EOFChar  = %d", dcbSerialParams.EofChar);
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
			DCS::Utils::Logger::Error("Serial: SetCommTimeouts() error. Using no values for timeouts.");
		}
		else
		{
			DCS::Utils::Logger::Debug("Serial: Set port timeouts successfull.");
		}
	}

	DCS::Utils::Logger::Debug("Serial: Connection initialized successfully.");

	return hComm;
}

BOOL DCS::Serial::write_bytes(HANDLE hComm, LPCSTR charArray, DWORD NbytesToWrite)
{
	//Check if the handle is in fact valid
	if (hComm == INVALID_HANDLE_VALUE)
	{
		DCS::Utils::Logger::Critical("Serial: HANDLE %u is invalid.\n", (uintptr_t)hComm);
		return FALSE;
	}

	DWORD NbytesWritten = 0;

	//Tx the array to the device
	BOOL status = WriteFile(hComm, charArray, NbytesToWrite, &NbytesWritten, NULL);

	if (status == FALSE)
	{
		DCS::Utils::Logger::Critical("Serial: Tx of data failed.");
		DCS::Utils::Logger::Critical("		Serial port = %u", (uintptr_t)hComm);
		DCS::Utils::Logger::Critical("		Serial data = %s", charArray);
		DCS::Utils::Logger::Critical("		Error       = %u", GetLastError());
		return FALSE;
	}

	DCS::Utils::Logger::Debug("Serial: Tx of data succeded. [%ull bytes written]", NbytesWritten);

	return TRUE;

}

BOOL DCS::Serial::read_bytes(HANDLE hComm, LPTSTR buffer, DWORD bufferSize, LPDWORD readBufferSize)
{
	//Check if the handle is in fact valid
	if (hComm == INVALID_HANDLE_VALUE)
	{
		DCS::Utils::Logger::Critical("Serial: HANDLE %u is invalid.", (uintptr_t)hComm);
		return FALSE;
	}

	DWORD dwEventMask = 0;
	char  localSerialBuffer[SERIAL_DEFAULT_READ_BUFFER_SIZE];

	//Receive any byte
	BOOL status = SetCommMask(hComm, EV_RXCHAR);

	if (status == FALSE)
	{
		DCS::Utils::Logger::Critical("Serial: SetCommMask() error. Event %d might be invalid.", EV_RXCHAR);
		DCS::Utils::Logger::Critical("Serial: Extended error: %u", GetLastError());
		return FALSE;
	}

	//Wait for the data
	status = WaitCommEvent(hComm, &dwEventMask, NULL);

	if (status == FALSE)
	{
		DCS::Utils::Logger::Critical("Serial: WaitCommEvent() error.");
		DCS::Utils::Logger::Critical("Serial: Extended error: %u", GetLastError());
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
		DCS::Utils::Logger::Error("Serial: Rx of data incomplete.");
		DCS::Utils::Logger::Error("		Read Buffer Size = %d", SERIAL_DEFAULT_READ_BUFFER_SIZE);
		DCS::Utils::Logger::Error("		Received Size    = %d", excess + i);
		DCS::Utils::Logger::Error("		Ignoring Size    = %d", excess);
		DCS::Utils::Logger::Error("		Received Bytes   = %s", localSerialBuffer);
		DCS::Utils::Logger::Error("		Terminated       = %s", "False");
		return FALSE;
	}

	if (status == FALSE)
	{
		DCS::Utils::Logger::Error("Serial: Rx of data failed.");
		DCS::Utils::Logger::Error("Serial: Extended error: %u", GetLastError());
		DCS::Utils::Logger::Error("		Read Buffer Size = %d", SERIAL_DEFAULT_READ_BUFFER_SIZE);
		DCS::Utils::Logger::Error("		Received Size    = %d", excess + i);
		DCS::Utils::Logger::Error("		Ignoring Size    = %d", excess);
		DCS::Utils::Logger::Error("		Received Bytes   = %s", localSerialBuffer);
		DCS::Utils::Logger::Error("		Terminated       = %s", localSerialBuffer[last] == '\n' ? "True" : "False");
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
		DCS::Utils::Logger::Error("Serial: Rx copy of data failed.\n");
		DCS::Utils::Logger::Error("Serial: Size of user buffer is too small.\n");
		DCS::Utils::Logger::Error("		Read Buffer Size = %d\n", SERIAL_DEFAULT_READ_BUFFER_SIZE);
		DCS::Utils::Logger::Error("		User Buffer Size = %d\n", bufferSize);
		return FALSE;
	}

	DCS::Utils::Logger::Debug("Serial: Rx of data succeded. [%d bytes read]", last);

	return TRUE;
}

BOOL DCS::Serial::close_handle(HANDLE hComm)
{
	DCS::Utils::Logger::Debug("Serial: Closing handle %x", hComm);
	return CloseHandle(hComm);
}
