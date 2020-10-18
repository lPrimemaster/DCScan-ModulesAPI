#pragma once
#include "DCS_ModuleUtils.h"

DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj = nullptr);
void FreeGenericHandle(DCS::GenericHandle hnd);