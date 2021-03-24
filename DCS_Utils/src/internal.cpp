#include "../include/internal.h"
#include <Windows.h>

DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj)
{
	DCS::GenericHandle hnd = malloc(size);
	if (!hnd)
	{
		LOG_ERROR("Could not allocate GenericHandle. Maybe ran out of memory?");
		return nullptr;
	}
	memcpy(hnd, obj, size);

	LOG_DEBUG("Success allocating GenericHandle %x (size=%u)", obj, size);

	return hnd;
}

void FreeGenericHandle(DCS::GenericHandle hnd)
{
	if (hnd)
	{
		free(hnd);
		LOG_DEBUG("Success deallocating GenericHandle %x", hnd);
	}
}

void DCS::Utils::GetConsoleSize(int* x, int* y)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	*x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	*y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

// Assuming virtual terminal is activated
void DCS::Utils::SetConsoleMargins(int t, int b)
{
	std::cout << ("\x1b[" + std::to_string(t) + ";" + std::to_string(b) + "r").c_str();
}

void DCS::Utils::WriteConsoleLine(int b, const char* str)
{
	//std::cout << "\x1b\x7";
	std::cout << ("\x1b[" + std::to_string(b) + ";1f").c_str();
	std::cout << str;
	std::cout << "\x1b[f";
	//std::cout << "\x1b\x8";
}
