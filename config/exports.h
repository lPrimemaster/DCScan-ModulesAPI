#pragma once

/** @file */

#define DCS_REGISTER_CALL

#ifdef API_EXPORT
/**
 * \brief Defines the export interface acessible via the dll-interface.
 * \todo Do not define API for Cmake's STATIC lib configuration
 * \todo Use Boost.Python to export to python via ctypes
 */
//#ifdef __cplusplus
#define DCS_API __declspec(dllexport)
//#else
//#define DCS_API extern "C" __declspec(dllexport)
//#endif
#else
//#ifdef __cplusplus
/**
 * \brief Defines the export interface acessible via the dll-interface.
 * \todo Do not define API for Cmake's STATIC lib configuration
 */
#define DCS_API __declspec(dllimport)
//#else
//#define DCS_API extern "C" __declspec(dllimport)
//#endif
#endif

#ifdef ENABLE_TESTING
/**
 * \brief Defines the export interface acessible via the dll-interface applicable for internal functions only.
 * 
 * This is used to expose certain internal features for testing, 
 * declarations with this specifier will not be exported to the
 * final built shared library.
 * \todo Do not define API for Cmake's STATIC lib configuration
 */
#define DCS_INTERNAL_TEST DCS_API
#else
/**
 * \brief Defines the export interface acessible via the dll-interface applicable for internal functions only.
 *
 * This is used to expose certain internal features for testing,
 * declarations with this specifier will not be exported to the
 * final built shared library.
 * \todo Do not define API for Cmake's STATIC lib configuration
 */
#define DCS_INTERNAL_TEST
#endif
