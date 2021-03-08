#ifndef _DCS_NETWORK_H
#define _DCS_NETWORK_H

#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../config/registry.h"

#include <unordered_map>

/**
 * @file
 * \brief Exposes Network functionalities of the API to the end user.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2021/02/22$
 */

namespace DCS
{
	/**
	 * \brief %Network control using sockets.
	 * 
	 * \todo Make all the connections / sockets secure with simple RSA hashing or using openSSL TLS
	 */
	namespace Network
	{
		/**
		 * \brief A generic socket.
		 */
		typedef DCS::GenericHandle Socket;

		DCS_API void Init();

		DCS_API bool GetStatus();

		DCS_API void Destroy();

		/**
		 * \brief %Server handling features.
		 */
		namespace Server
		{
			/**
			 * \brief Dictates how the server thread should stop.
			 */
			enum class StopMode
			{
				IMMEDIATE, ///< Stop server thread without waiting for the client to disconnect (force shutdown server-side). 
				WAIT       ///< Stop server after waiting for the client to disconnect.
			};

			DCS_API Socket Create(i32 port);

			DCS_API Socket WaitForConnection(Socket server);

			DCS_API void StartThread(Socket client);

			DCS_API void StopThread(Socket client, StopMode mode = StopMode::IMMEDIATE);

			/**
			 * \example sockets/echo_server.cpp.
			 * An example showing how to implement a simple echo server.
			 */
		}

		/**
		 * \brief %Client handling features.
		 */
		namespace Client
		{
			DCS_API Socket Connect(DCS::Utils::String host, i32 port);

			DCS_API void StartThread(Socket connection);

			DCS_API void StopThread(Socket connection);

			/**
			 * \example sockets/echo_client.cpp.
			 * An example showing how to implement a simple echo client.
			 */
		}

		/**
		 * \brief Handles default messaging via a simple protocol.
		 *
		 * Messages sent via network can be of any type (for example JSON), however, the Messaging namespace provides
		 * a default simple messaging style used by the GUI when contacting the server. This implementation attempts
		 * to reduce the size of the sent messages, maximizing network functionality.
		 *
		 * If you wish to implement a custom messaging system to interact via a script refer to: (TODO)
		 *
		 * Despite the means how the message shall be carried, it always needs to be converted to the only server-side
		 * accepted struct, which is responsible for knowing which operation to perform
		 * on the server-side.
		 *
		 * \todo Finish this documentation, use a custom msg read callback
		 */
		namespace Message
		{
			/**
			 * \brief Defines the message operation modes in the tcp/ip connection.
			 */
			enum class DCS_API Operation
			{
				NO_OP = 0,			///< Ping the server only.
				REQUEST = 1,		///< Request a function call to the server.
									// 2 and 3 reserved
				RESPONSE = 4,		///< Send back a response to the client.
									// 5 and 6 reserved
				SUB_EVT = 7,		///< Subscribe to a server-side event.
				UNSUB_EVT,			///< Unsubscribe from a previously subscribed event.
				DATA				///< Send or receive data only.
			};

			DCS_API void SendAsync(Operation op, u8* data, i32 size);
			DCS_API Registry::SVReturn SendSync(Operation op, u8* data, i32 size);
		}
	}
}

#endif _DCS_NETWORK_H
