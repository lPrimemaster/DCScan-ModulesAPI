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

		/**
		 * \brief Initializes the %Network module.
		 */
		DCS_API void Init();

		/**
		 * \brief Check if the %Network module is initilized.
		 */
		DCS_API bool GetStatus();

		/**
		 * \brief Destroys the %Network module.
		 */
		DCS_API void Destroy();

		/**
		 * \brief %Server-side handling features.
		 */
		namespace Server
		{
			/**
			 * \brief Dictates how the server thread should stop.
			 */
			enum class StopMode
			{
				IMMEDIATE, ///< Stop server thread without waiting for the client to disconnect (force shutdown server-side). 
				WAIT       ///< Stop server after waiting for the client to disconnect (blocking the calling thread).
			};

			/**
			 * \brief Create server listening port.
			 * \param port Port to bind the server.
			 */
			DCS_API Socket Create(i32 port);

			/**
			 * \brief Wait for a client to connect to the server connection Socket.
			 * \param server created server socket to wait on.
			 */
			DCS_API Socket WaitForConnection(Socket server);

			/**
			 * \brief Starts the server thread on a established connection.
			 * \param client The connection between server and client returned by WaitForConnection(Socket).
			 */
			DCS_API void StartThread(Socket client);


			/**
			 * \brief Stops the server thread on a established connection (Forces disconnect).
			 * \param client The connection between server and client that is currently active.
			 * \param mode How the server thread stops (view StopMode).
			 */
			DCS_API void StopThread(Socket client, StopMode mode = StopMode::IMMEDIATE);

			/**
			 * \example sockets/simple_server.cpp.
			 * An example showing how to implement a simple server.
			 */
		}

		/**
		 * \brief %Client-side handling features.
		 */
		namespace Client
		{
			/**
			 * \brief Connect to a running server.
			 * \param host Server IPv4 address.
			 * \param port Server listening port.
			 */
			DCS_API Socket Connect(DCS::Utils::String host, i32 port);

			/**
			 * \brief Starts the client thread on a established connection.
			 * \param connection The connection between server and client returned by Connect(DCS::Utils::String, i32).
			 */
			DCS_API void StartThread(Socket connection);

			/**
			 * \brief Stops the client thread on a established connection (Forces disconnect).
			 * \param connection The connection between server and client that is currently active.
			 */
			DCS_API void StopThread(Socket connection);

			/**
			 * \brief Gets the current connection latency in milliseconds.
			 * Possible last check: 10 seconds ago
			 * Interval: 0 - 1000 ms
			 */
			DCS_API i16 GetMillisLatency();

			/**
			 * \example sockets/simple_client.cpp.
			 * An example showing how to implement a simple client.
			 */
		}

		/**
		 * \brief Handles default messaging via a simple fixed protocol.
		 * 
		 * If you wish to send a custom message not defined via this protocol, it is possible to implement on top
		 * of it and sending pure data to the server/client via the Message::Operation::DATA token. Sending data is async
		 * and uses custom DataIn/DataOut user defined callbacks to handle the binary data. The default max message size is 4096 bytes
		 * due to the custom FIFO queue max memory size (which can be changed).
		 * 
		 * Sending default messages is easy through the use of the Send*(Operation, u8*, i32) functions.
		 * \see Message::SendAsync(Operation, u8*, i32)
		 * \see Message::SendSync(Operation, u8*, i32)
		 * 
		 * All the data is assumed to be in the same byte-order as the machine. As tought this code will not work if host and client
		 * are of different endianness. Being most systems x86_64 this should not pose a problem for a simple system.
		 * 
		 * \todo Convert everything to network byte-order (big endian)
		 * 
		 * \todo Update socket examples
		 */
		namespace Message
		{
			/**
			 * \brief Defines the message operation modes in the tcp/ip connection.
			 */
			enum class Operation
			{
				NO_OP = 0,			///< Do nothing.
				REQUEST = 1,		///< Request a function call to the server.
									// 2 and 3 reserved
				RESPONSE = 4,		///< Send back a response to the client.
									// 5 and 6 reserved
				EVT_SUB = 7,		///< Subscribe to a server-side event.
									// 8 reserved
				EVT_UNSUB = 9,		///< Unsubscribe from a previously subscribed event.
				DATA				///< Send or receive data only.
			};

			/**
			 * \brief Sends an asynchronous message to the server.
			 * 
			 * \see DCS::Registry
			 * 
			 * \param op The type of operation to perform server-side.
			 * \param data The DefaultMessage formated message. Expects diferent data deppending on the operation.
			 * \param size Size of the data bytes.
			 */
			DCS_API void SendAsync(Operation op, u8* data, i32 size);

			/**
			 * \brief Sends a synchronous message to the server.
			 * 
			 * \see DCS::Registry
			 * 
			 * \param op The type of operation to perform server-side.
			 * \param data The DefaultMessage formated message. Expects diferent data deppending on the operation.
			 * \param size Size of the data bytes.
			 * 
			 * \return Waits, blocking the calling thread, for the server response (see Registry::SVReturn).
			 */
			DCS_API Registry::SVReturn SendSync(Operation op, u8* data, i32 size);


			DCS_REGISTER_EVENT(OnTestFibSeq)
			DCS_API void FibSeqEvt();
		}
	}
}

#endif _DCS_NETWORK_H
