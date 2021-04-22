#pragma once

/**
 * @file
 */

/**
 * \defgroup calls  Remote Server Callables
 * \brief A module containing all the API functions callable via TCP/IP.
 * 
 * An example syntax to call a function is:
 * \code
 * unsigned char buffer[1024];
 * 
 * // Register the message
 * auto size_written = DCS::Registry::SVParams::GetDataFromParams(buffer,
 *		SV_CALL_FUNC_NAME_WITH_SCOPE,
 *		3, DCS::Utils::BasicString{ "Hello server!" }
 *	);
 *	
 * // Send the message and wait for response
 * auto ret_val = Message::SendSync(Message::Operation::REQUEST, buffer, size_written);
 * // Or dont wait...
 * Message::SendAsync(Message::Operation::REQUEST, buffer, size_written);
 * \endcode
 */

/**
 * \defgroup events Remote Server Events
 * \brief A module containing all the API events subscribable via TCP/IP.
 * 
 * An example syntax to sub/unsub to events is:
 * \code
 * unsigned char buffer[1024];
 * 
 * // Register the event
 * auto size_written = DCS::Registry::SetupEvent(buffer, SV_EVT_OnTestFibSeq, [] (DCS::u8* data) {
 *		LOG_DEBUG("FibEvent returned: %llu", *(DCS::u64*)data);
 * });
 * 
 * // Send the sub request
 * Message::SendAsync(Message::Operation::EVT_SUB, buffer, size_written);
 * 
 * // ...
 * 
 * // Unsubscribe
 * size_written = DCS::Registry::RemoveEvent(buffer, SV_EVT_OnTestFibSeq);
 * Message::SendAsync(Message::Operation::EVT_UNSUB, buffer, size_written);
 * \endcode
 */

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

 /**
  * \brief Append this definition before function declarations (or anywhere in the code) to register a tcp connection
  * event.
  * 
  * After running the DCS Preprocessor, integer codes should be generated for each
  * event registered.
  *
  * Examples:
  * \code{.cpp}
  * DCS_REGISTER_EVENT
  * void onData(int x);
  * 
  * void onData(int x)
  * {
  *		// Emits the the SV_EVT_onData event, passing param x everytime the function onData is called.
  *		DCS_EMIT_EVT((u8*)&x, sizeof(int));
  * }
  * \endcode
  */
#define DCS_REGISTER_EVENT

#ifdef SHARED
#ifdef API_EXPORT
/**
 * \brief Defines the export interface acessible via the dll-interface.
 * Only important for SHARED LIB  Compile Mode.
 */
#define DCS_API __declspec(dllexport)
#else
/**
 * \brief Defines the export interface acessible via the dll-interface.
 * Only important for SHARED LIB  Compile Mode.
 */
#define DCS_API __declspec(dllimport)
#endif
#else
/**
 * \brief Defines the export interface acessible via the dll-interface.
 * Only important for SHARED LIB  Compile Mode.
 */
#define DCS_API
#endif


#ifdef ENABLE_TESTING
/**
 * \brief Defines the export interface acessible via the dll-interface applicable for internal functions only.
 * 
 * This is used to expose certain internal features for testing, 
 * declarations with this specifier will not be exported to the
 * final built shared library.
 */
#define DCS_INTERNAL_TEST DCS_API
#else
/**
 * \brief Defines the export interface acessible via the dll-interface applicable for internal functions only.
 *
 * This is used to expose certain internal features for testing,
 * declarations with this specifier will not be exported to the
 * final built shared library.
 */
#define DCS_INTERNAL_TEST
#endif
