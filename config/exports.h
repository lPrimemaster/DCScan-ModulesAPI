#pragma once
//#ifdef API_EXPORT
//
//#ifdef __cplusplus
//#define DCS_API_C extern "C" __declspec(dllexport)
//#else
//#define DCS_API_C
//#endif
//#else
//#ifdef __cplusplus
//#define DCS_API_C extern "C" __declspec(dllimport)
//#else
//#define DCS_API_C
//#endif
//#endif

/** @file */

#ifdef API_EXPORT
/**
 * \brief Defines the export interface acessible via the dll.
 * \todo Do not define API for Cmake's STATIC lib configuration
 */
//#ifdef __cplusplus
#define DCS_API __declspec(dllexport)
//#else
//#define DCS_API extern "C" __declspec(dllexport)
//#endif
#else
//#ifdef __cplusplus
/**
 * \brief Defines the export interface acessible via the dll.
 * \todo Do not define API for Cmake's STATIC lib configuration
 */
#define DCS_API __declspec(dllimport)
//#else
//#define DCS_API extern "C" __declspec(dllimport)
//#endif
#endif

#ifdef ENABLE_TESTING
#define DCS_INTERNAL_TEST DCS_API
#else
#define DCS_INTERNAL_TEST
#endif
