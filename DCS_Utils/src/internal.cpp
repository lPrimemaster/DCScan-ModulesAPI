#include "../include/internal.h"

DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj)
{
	DCS::GenericHandle hnd = malloc(size);
	if (!hnd)
	{
		///\todo LOG ERROR
		return nullptr;
	}
	memcpy(hnd, obj, size);
	return hnd;
}

void FreeGenericHandle(DCS::GenericHandle hnd)
{
	if (hnd)
	{
		free(hnd);
	}
}
