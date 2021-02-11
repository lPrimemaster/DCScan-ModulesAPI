#include "../include/internal.h"

DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj)
{
	DCS::GenericHandle hnd = malloc(size);
	if (!hnd)
	{
		DCS::Utils::Logger::Error("Could not allocate GenericHandle. Maybe ran out of memory?");
		return nullptr;
	}
	memcpy(hnd, obj, size);

	DCS::Utils::Logger::Debug("Success allocating GenericHandle %x (size=%u)", obj, size);

	return hnd;
}

void FreeGenericHandle(DCS::GenericHandle hnd)
{
	if (hnd)
	{
		free(hnd);
		DCS::Utils::Logger::Debug("Success deallocating GenericHandle %x", hnd);
	}
}
