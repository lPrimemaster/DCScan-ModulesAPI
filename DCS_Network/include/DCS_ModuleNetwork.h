#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

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
		 * \brief The callback prototype to use on Server::StartThread(Socket, OnDataReceivedCallback).
		 */
		typedef void (*OnDataReceivedCallback)(const char* data, DCS::i64 size, Socket client);

		/**
		 * \brief %Server handling features.
		 */
		namespace Server
		{
			/**
			 * \brief Creates a server Socket with a port.
			 * 
			 * \param port The Socket listen port.
			 * \return Server Socket.
			 */
			DCS_API Socket Create(DCS::i32 port);


			/**
			 * \brief Waits for a client connection on a valid server Socket, 
			 * blocking code execution on the running thread.
			 * 
			 * The server Socket is destroyed after the client connects.
			 * Only one client connection is allowed at a given time.
			 * 
			 * \param server The server Socket.
			 * 
			 * \return Socket containing the client connection.
			 */
			DCS_API Socket WaitForConnection(Socket server);

			/**
			 * \brief Starts the thread for the server and waits for client data.
			 * 
			 * The client is responsible for making the first request.
			 * Data can be sent via the OnDataReceivedCallback callback function (see SendData(Socket, const char*, DCS::i64))
			 * 
			 * \param client The client connection Socket.
			 * \param drc The function to be called when a full message is received. Expects text data.
			 * A lambda expression can be used as the callback, provided it does not capture.
			 */
			DCS_API void StartThread(Socket client, OnDataReceivedCallback drc);

			/**
			 * \brief Stops the thread for the server and shuts down the socket.
			 *
			 * \param client The client connection Socket to close.
			 */
			DCS_API void StopThread(Socket client);

			/**
			 * \brief Sends data to the client.
			 * \todo implement
			 * \param client Client Socket.
			 * \param data Text/binary data to send.
			 * \param size Size of the data sent.
			 */
			DCS_API void SendData(Socket client, const char* data, DCS::i64 size);

			/**
			 * \example sockets/echo_server.cpp.
			 * An example showing how to implement a simple echo server.
			 */
		}
	}
}
