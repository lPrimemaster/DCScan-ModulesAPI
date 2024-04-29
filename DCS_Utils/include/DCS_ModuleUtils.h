#ifndef _DCS_UTILS_H
#define _DCS_UTILS_H

#pragma once
#include "../../config/exports.h"
#include <iostream>
#include <chrono>
#include <mutex>
#include <map>
#include <string>
#include <future>

/**
 * @file
 * 
 * \brief Container for all the exported symbols that fall in the Utils category.
 * 
 * Also contains library common code.
 * 
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date 2020/10/16
 */


#ifdef SOURCE_PATH_SIZE
#define LOG_LVL(lvl, msg, ...) DCS::Utils::Logger::lvl(__FILE__ + SOURCE_PATH_SIZE, msg, ##__VA_ARGS__) ///< Alias to DCS::Utils::Logger::lvl(__FILE__ + SOURCE_PATH_SIZE, msg, __VA_ARGS__)
#else 
#define LOG_LVL(lvl, msg, ...) DCS::Utils::Logger::lvl(__FILE__, msg, ##__VA_ARGS__) ///< Alias to DCS::Utils::Logger::lvl(__FILE__, msg, __VA_ARGS__)
#endif

#define LOG_DEBUG(msg, ...) LOG_LVL(Debug, msg, ##__VA_ARGS__) ///< Alias to LOG_LVL(Debug, msg, __VA_ARGS__)
#define LOG_MESSAGE(msg, ...) LOG_LVL(Message, msg, ##__VA_ARGS__) ///< Alias to LOG_LVL(Message, msg, __VA_ARGS__)
#define LOG_WARNING(msg, ...) LOG_LVL(Warning, msg, ##__VA_ARGS__) ///< Alias to LOG_LVL(Warning, msg, __VA_ARGS__)
#define LOG_ERROR(msg, ...) LOG_LVL(Error, msg, ##__VA_ARGS__) ///< Alias to LOG_LVL(Error, msg, __VA_ARGS__)
#define LOG_CRITICAL(msg, ...) LOG_LVL(Critical, msg, ##__VA_ARGS__) ///< Alias to LOG_LVL(Critical, msg, __VA_ARGS__)

#define ENUM_FLAG_OPERATOR(T,X) inline T operator X (T lhs, T rhs) { return (T) (static_cast<std::underlying_type_t <T>>(lhs) X static_cast<std::underlying_type_t <T>>(rhs)); } 
#define ENUM_FLAGS(T) \
enum class T; \
inline T operator ~ (T t) { return (T) (~static_cast<std::underlying_type_t <T>>(t)); } \
ENUM_FLAG_OPERATOR(T,|) \
ENUM_FLAG_OPERATOR(T,^) \
ENUM_FLAG_OPERATOR(T,&) \
enum class T

namespace DCS
{
	typedef signed long long i64; ///< Equivalent to int64_t.
	typedef unsigned long long u64; ///< Equivalent to uint64_t.
	typedef signed long i32; ///< Equivalent to int32_t.
	typedef unsigned long u32; ///< Equivalent to uint32_t.
	typedef short i16; ///< Equivalent to int16_t.
	typedef unsigned short u16; ///< Equivalent to uint16_t.
	typedef signed char i8; ///< Equivalent to int8_t.
	typedef unsigned char u8; ///< Equivalent to uint8_t.

	typedef float f32; ///< Equivalent to float.
	typedef double f64; ///< Equivalent to double.

	/**
	 * \brief A generic opaque handle that is only meaningful for the API.
	 */
	typedef void* GenericHandle;

	namespace Utils
	{
		/**
		 * \brief A wrapper class used to export a simple allocatable vector to the client side.
		 * Similar to std::vector<T>
		 * \tparam T the type to create the vector out of.
		 */
		template<typename T>
		class DCS_API Vector
		{
		public:
			Vector() = default;
			~Vector()
			{
				destroy();
			}
			Vector(Vector<T>&& vector)
			{
				destroy();
				vector.data = data;
				vector.max_size = max_size;
				vector.end_offset = end_offset;
			}
			Vector(const Vector<T>& vector)
			{
				destroy();
				data = (T*)malloc(vector.max_size * sizeof(T));
				memcpy(data, vector.data, vector.end_offset * sizeof(T));
				max_size = vector.max_size;
				end_offset = vector.end_offset;
			}

			/**
			 * \brief Reserve size for the Vector<T>.
			 * \param count count * sizeof(T) size to reserve.
			 */
			inline void reserve(size_t count)
			{
				reallocate(count);
			}

			/**
			 * \brief Push value into the Vector<T>. Similar to std::vector::push_back.
			 * \param value Value to push.
			 */
			inline void push(const T& value)
			{
				emplace(value);
			}

			/**
			 * \brief Push value into the Vector<T>. Similar to std::vector::push_back.
			 * \param value Value to push.
			 */
			inline void push(T&& value)
			{
				emplace(std::move(value));
			}

			/**
			 * \brief Emplace value into the Vector<T>. Similar to std::vector::emplace_back.
			 * \param args Constructor arguments.
			 */
			template<typename... Args>
			inline void emplace(Args&&... args)
			{
				if(end_offset >= max_size)
				{
					reallocate();
				}
				new (data + end_offset++) T(std::forward<Args>(args)...);
			}

			/**
			 * \brief Get value at position.
			 * \param i Index.
			 */
			inline const T& operator[](size_t i) const
			{
				return data[i];
			}

			/**
			 * \brief Get value at position.
			 * \param i Index.
			 */
			inline T& operator[](size_t i)
			{
				return data[i];
			}

			/**
			 * \brief Get vector size. Similar to std::vector::size.
			 */
			inline size_t size() const
			{
				return end_offset;
			}

			/**
			 * \brief Get vector capacity. Similar to std::vector::capacity.
			 */
			inline size_t capacity() const
			{
				return max_size;
			}

			inline T* begin()
			{
				return data;
			}

			inline T* end()
			{
				return data + end_offset;
			}

		private:
			inline void destroy()
			{
				for(size_t i = 0; i < end_offset; i++)
				{
					data[i].~T();
				}

				if(data != nullptr)
				{
					free(data);
					data = nullptr;
				}
			}

			inline void reallocate(size_t sz = 0)
			{
				size_t alloc_sz;
				if(data == nullptr)
				{
					alloc_sz = 1;
				}
				else if(sz == 0)
				{
					alloc_sz = max_size * 2;
				}
				else if(sz > max_size)
				{
					alloc_sz = sz;
				}
				else
				{
					return;
				}

				data = (T*)realloc(data, alloc_sz * sizeof(T));
				max_size = alloc_sz;

				if(data == nullptr)
				{
					// LOG_CRITICAL("Failed to allocate memory for Vector<T>.");
				}
			}

			T* data = nullptr;
			size_t max_size = 0;
			size_t end_offset = 0;
		};

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

			/**
			 * \brief Create a String object from pointer and size.
			 */
			String(const char* ptr, const size_t size);

			String(const String& s);

			~String();

			String& operator=(const String& s) noexcept;
			
			/**
			 * \brief Split a string.
			 */
			Vector<String> split(const char separator = ' ');

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

			inline const f64 tof64() const
			{
				return std::atof(buffer);
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
		 * \brief A very simple string buffer to hold char values.
		 * Used for string information manipulations via tcp/ip mostly.
		 */
		struct DCS_API BasicString
		{
			char buffer[512]; ///< The sized string buffer.
		};

		/**
		 * \brief This class enables writing to a single (or multiple) buffer(s) for logging.
		 * Thread-safe.
		 * 
		 * \todo Make the output redirectable (for stderr or stdout for example).
		 */
		class Logger
		{
		public:
			enum class Verbosity : int;
		private:
			
			typedef void (*WriteNotifyCallback)(DCS::Utils::String string, Verbosity v, void*);

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
			static DCS_API void Debug(const char* file, const char* msg, ...);

			/**
			 * \brief Emit a status message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Message(const char* file, const char* msg, ...);

			/**
			 * \brief Emit a warning message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Warning(const char* file, const char* msg, ...);

			/**
			 * \brief Emit an error message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Error(const char* file, const char* msg, ...);

			/**
			 * \brief Emit a critical error message.
			 * 
			 * Uses printf like formating.
			 */
			static DCS_API void Critical(const char* file, const char* msg, ...);

			/**
			 * \brief Changes Logger Options.
			 */
			static DCS_API void Settings(Options opt);

			/**
			 * \brief Changes the verbosity level. 
			 */
			static DCS_API void ChangeVerbosity(Verbosity v);

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

		/**
		 * \brief A class that holds possible future data.
		 * 
		 * To be used as a return type when expected data T is returned async.
		 * 
		 * \tparam T the type to manipulate.
		 */
		template<typename T>
		class AsyncItem
		{
		public:
			AsyncItem(std::promise<T>& p) { f = p.get_future(); };
			~AsyncItem() = default;

			/**
			 * \brief Wait for item to be available.
			 */
			inline void wait() const
			{
				f.wait();
			}

			/**
			 * \brief Get the item stored in the future.
			 * 
			 * If item is not available, it waits until it is.
			 * 
			 * \return T item
			 */
			inline T get() const
			{
				return f.get();
			}

		private:
			std::shared_future<T> f;
		};
	}

	/**
	 * \brief Timing related utilities.
	 */
	namespace Timer
	{
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
	}
}

#endif //_DCS_UTILS_H
