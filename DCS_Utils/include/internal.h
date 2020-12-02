#pragma once
#include "DCS_ModuleUtils.h"

/**
 * @file
 * \internal
 * \brief Exposes internal functionalities of the Utils namespace.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2020/11/01$
 */

/**
 * \internal
 * \brief Creates a opaque generic handle only usefull to the API.
 * 
 * This is usefull for passing STL pointers to the client to hold API states.
 */
DCS::GenericHandle AllocateGenericHandle(DCS::u16 size, DCS::GenericHandle obj = nullptr);

/**
 * \internal
 * \brief Frees a generic handle's memory.
 */
void FreeGenericHandle(DCS::GenericHandle hnd);