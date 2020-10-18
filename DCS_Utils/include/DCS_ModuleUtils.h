#pragma once
#include "../../config/exports.h"
#include <iostream>
#include <chrono>

/**
 * @file
 * 
 * \brief Container for all the exported symbols that fall in the Utils category.
 * 
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date 2020/10/16
 */

namespace DCS
{
	typedef signed long long i64; ///< Equivalent to int_64t.
	typedef unsigned long long u64; ///< Equivalent to uint_64t.
	typedef signed long i32; ///< Equivalent to int_32t.
	typedef unsigned long u32; ///< Equivalent to uint_32t.
	typedef signed i16; ///< Equivalent to int_16t.
	typedef unsigned u16; ///< Equivalent to uint_16t.
	typedef signed char i8; ///< Equivalent to int_8t.
	typedef unsigned char u8; ///< Equivalent to uint_8t.

	typedef float f32; ///< Equivalent to float.
	typedef double f64; ///< Equivalent to double.

	typedef void* GenericHandle;

	namespace Utils
	{
		/**
		 * \brief This class enables writing to a single (or multiple) buffer(s) for logging.
		 * Thread-safe.
		 */
		class DCS_API Logger
		{
			static void Debug(const char* msg);
			static void Message(const char* msg);
			static void Warning(const char* msg);
			static void Error(const char* msg);
			static void Critical(const char* msg);
		};


		/**
		 * \brief A wrapper class used to export a simple string to the client side.
		 */
		class DCS_API String
		{
		public:
			String() = default;
			String(const char* text);
			~String();

			inline const size_t size() const
			{
				return buffer_size;
			}

			inline const char* c_str() const
			{
				return buffer;
			}

		private:
			size_t buffer_size = 0;
			char* buffer = nullptr;
		};
	}

	namespace Timer
	{
		/**
		 * \brief Holds data about when a timer was first created.
		 *
		 * Used to get relative timestamps with 100 nanoseconds precision.
		 */
		struct DCS_API SystemTimer
		{
			GenericHandle point;
		};

		/**
		 * \brief Represents a timestamp divided fieldwise.
		 */
		struct DCS_API Timestamp
		{
			i16 hour = 0;
			i16 min = 0;
			i16 sec = 0;
			i16 millis = 0;
			i16 micros = 0;
			i16 nanos = 0;

			/**
			 * \brief Converts a Timestamp to string format.
			 * \return Utils::String Timestamp string formated.
			 */
			const Utils::String to_string() const;
		};

		/**
		 * \brief Creates a new SystemTimer.
		 */
		DCS_API SystemTimer New();

		/**
		 * \brief Deletes a SystemTimer.
		 */
		DCS_API void Delete(SystemTimer timer);

		/**
		 * \brief Gives a timestamp relative to timer in Timestamp format.
		 * 
		 * \param timer Relative point to measure.
		 * \return Timestamp.
		 */
		DCS_API Timestamp GetTimestamp(SystemTimer timer);

		/**
		 * \brief Gives a timestamp relative to timer in Utils::String format.
		 *
		 * \param timer Relative point to measure.
		 * \return Utils::String timestamp [XXh XXm XXs XXms XXus XXns].
		 */
		DCS_API Utils::String GetTimestampString(SystemTimer timer);

		/**
		 * \brief Gives number of nanoseconds passed relative to timer.
		 *
		 * \param timer Relative point to measure.
		 * \return Number of nanoseconds stored in a DCS::i64.
		 */
		DCS_API i64 GetNanoseconds(SystemTimer timer);
	}

}