#pragma once
#include "../include/DCS_ModuleEngineControl.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

#include <Windows.h>

/**
 * @file
 * \internal
 * \brief Internal serial COM TX/RX.
 *
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date 2020/11/26
 * $Modified: 2020/10/19$
 */


/**
 * \brief Serial Control.
 */
namespace serial
{
	/**
	 * \internal
	 * \brief All serial arguments for windows manipulation.
	 */
	struct DCS_INTERNAL_TEST SerialArgs
	{
		DWORD baudRate;
		BYTE byteSize;
		BYTE stopBits;
		BYTE parity;

		CHAR eofChar;

		DWORD readIntervalTimeout;
		DWORD readTotalTimeoutConstant;
		DWORD readTotalTimeoutMultiplier;
		DWORD writeTotalTimeoutConstant;
		DWORD writeTotalTimeoutMultiplier;
	};

	/**
	 * \internal
	 * \brief Initialized COM HANDLE with params.
	 */
	DCS_INTERNAL_TEST HANDLE init_handle(LPCSTR portName, DWORD rwAccess, SerialArgs args);

	/**
	 * \internal
	 * \brief Writes to COM HANDLE with params.
	 */
	DCS_INTERNAL_TEST BOOL   write_bytes(HANDLE hComm, LPCSTR charArray, DWORD NbytesToWrite);

	/**
	 * \internal
	 * \brief Reads from COM HANDLE with params.
	 */
	DCS_INTERNAL_TEST BOOL   read_bytes(HANDLE hComm, LPTSTR buffer, DWORD bufferSize, LPDWORD readBufferSize);

	/**
	 * \internal
	 * \brief Closes COM HANDLE.
	 */
	DCS_INTERNAL_TEST BOOL   close_handle(HANDLE hComm);
}