#pragma once

/** @file */

/**
 * \brief Append this definition before function declarations to register it as a tcp connection
 * callable. First param is the return type, the following params are argument types (if existent).
 * 
 * After running the DCS Preprocessor, integer codes (as well as macro definitions) should be generated for each
 * function registered.
 * 
 * Note that the parameters name should be written in full, regardless of the current scope.
 * See last example.
 * 
 * Examples:
 * \code{.cpp}
 * DCS_REGISTER_CALL(void, int)
 * void funcInt(int x);
 * 
 * DCS_REGISTER_CALL(void)
 * void func();
 * 
 * DCS_REGISTER_CALL(int, DCS::i8)
 * int DCS::example_func(i8 val);
 * \endcode
 */
#define DCS_REGISTER_CALL(...)

#define DCS_REGISTER_EVENT(name)

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
