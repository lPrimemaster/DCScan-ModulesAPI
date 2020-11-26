#pragma once
#include "DCS_ModuleUtils.h"

/**
 * @file
 * \brief Exposes internal functionalities of the Utils namespace.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2020/11/01$
 */

DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj = nullptr);
void FreeGenericHandle(DCS::GenericHandle hnd);