#pragma once
#ifdef API_EXPORT
/// \todo Do not define API for Cmake's STATIC lib configuration
#define DCS_API __declspec(dllexport)
#else
#define DCS_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Provide an example
 *
 * This class is meant as an example. It is not useful by itself
 * rather its usefulness is only a function of how much it helps
 * the reader.  It is in a sense defined by the person who reads it
 * and otherwise does not exist in any real form.
 *
 * \author César Godinho
 *
 * \version 1.0 $Revision: 1.5 $
 *
 * \date $Date: 2020/10/12$
 */
DCS_API int power2(int x);

#ifdef __cplusplus
}
#endif
