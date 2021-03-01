#include "../include/internal.h"

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
