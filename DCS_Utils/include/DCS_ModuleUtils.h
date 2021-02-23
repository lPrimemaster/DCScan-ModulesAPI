#pragma once
#include "../../config/exports.h"
#include <iostream>
#include <chrono>
#include <mutex>
#include <map>
#include <string>

/**
 * @file
 * 
 * \brief Container for all the exported symbols that fall in the Utils category.
 * 
 * Also contains library common code.
 * 
 * \todo Make the String class callable with a C-interface (for python eg.)
 * 
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date 2020/10/16
 * $Modified: 2020/10/19$
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

	/**
	 * \brief A generic opaque handle that is only meaningful for the API.
	 * 
	 * \todo Maybe change this to intptr?
	 */
	typedef void* GenericHandle;

	namespace Utils
	{
		/**
		 * \brief A wrapper class used to export a simple string to the client side.
		 */
		class DCS_API String
		{
		public:
			String() = default;

			/**
			 * \brief Create a String object from pointer.
			 */
			String(const char* text);
			String(const String& s);

			~String();

			/**
			 * \brief Returns size of the string
			 * \return str size.
			 */
			inline const size_t size() const
			{
				return buffer_size;
			}

			template<typename T>
			static inline const String From(T val)
			{
				return String(std::to_string(val).c_str());
			}

			/**
			 * \brief Converts a String object to const char* raw pointer
			 * \return buffer.
			 */
			inline const char* c_str() const
			{
				return buffer;
			}

		private:
			size_t buffer_size = 0;
			char* buffer = nullptr;
		};

		/**
		 * \brief This class enables writing to a single (or multiple) buffer(s) for logging.
		 * Thread-safe.
		 * 
		 * \todo Add Deinit static method.
		 * \todo Make the output redirectable (for stderr or stdout for example).
		 */
		class Logger
		{
		private:
			typedef void (*WriteNotifyCallback)(DCS::Utils::String string, void*);

		public:
			/**
			 * \brief Contains the possible verbosity levels for the Logger.
			 * 
			 * Passing a value to Logger::Init will prevent lower priority messages from being displayed in the console.
			 * The file output (Logger::handle) is not affected by this, printing every event.
			 */
			enum class Verbosity
			{
				CRITICAL, ///< Enables console output only for critical messages. 
				ERROR,    ///< Enables console output for error and higher priority messages. 
				WARNING,  ///< Enables console output for warning and higher priority messages. 
				MESSAGE,  ///< Enables console output for status and higher priority messages. 
				DEBUG     ///< Enables console output for debug and higher priority messages.
			};

			/**
			 * \brief Contains the possible Logger custom options.
			 */
			enum class Options
			{
				ENABLE_COLOR, ///< Enables console output with colors.
				DISABLE_COLOR ///< Disables console output with colors.
			};

			/**
			 * \brief Initialize the Logger system. Call only once.
			 */
			static DCS_API void Init(Verbosity level, DCS::Utils::String file = "default_log.log");

			/**
			 * \brief Destroy the Logger system. Call only once.
			 */
			static DCS_API void Destroy();

			/**
			 * \brief Sets the callback to be called when a log is made. Then sends the text to the client.
			 */
			static DCS_API void SetLogWriteCallback(WriteNotifyCallback wnc, void* obj = nullptr);

			/**
			 * \brief Emit a debug message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Debug(const char* msg, ...);

			/**
			 * \brief Emit a status message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Message(const char* msg, ...);

			/**
			 * \brief Emit a warning message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Warning(const char* msg, ...);

			/**
			 * \brief Emit an error message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Error(const char* msg, ...);

			/**
			 * \brief Emit a critical error message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Critical(const char* msg, ...);

			/**
			 * \brief Changes Logger Options.
			 */
			static DCS_API void Settings(Options opt);

		private:
			static void WriteData(std::string buffer[], Verbosity v);

			static WriteNotifyCallback writenotify;
			static void* obj;

			static Verbosity verbosity_level;
			static Options options;
			static std::string timestamp();
			static FILE* handle;
			static std::mutex _log_mtx;
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
			GenericHandle point; ///< Represents a time point.
		};

		/**
		 * \brief Represents a timestamp divided fieldwise.
		 */
		struct DCS_API Timestamp
		{
			i16 hour = 0; ///< Timestamp value: hours.
			i16 min = 0; ///< Timestamp value: minutes.
			i16 sec = 0; ///< Timestamp value: seconds.
			i16 millis = 0; ///< Timestamp value: milliseconds.
			i16 micros = 0; ///< Timestamp value: microseconds.
			i16 nanos = 0; ///< Timestamp value: nanoseconds.

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
