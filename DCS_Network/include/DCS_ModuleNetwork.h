#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

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
		 * \brief The callback prototype to use on Server::StartThread(Socket, OnDataReceivedCallback).
		 */
		typedef void (*OnDataReceivedCallback)(const unsigned char* data, DCS::i16 size, Socket client);

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

			/**
			 * \brief Creates a server Socket with a port.
			 * 
			 * \param port The Socket listen port.
			 * \return Server Socket.
			 */
			DCS_API Socket Create(i32 port);

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
			 * \param drc The function to be called when a full message is received. Expects text data terminated via \r\n (or \r\n\0).
			 * A lambda expression can be used as the callback, provided it does not capture.
			 */
			DCS_API void StartThread(Socket client, OnDataReceivedCallback drc);

			/**
			 * \brief Stops the thread for the server and shuts down the socket.
			 *
			 * \param client The client connection Socket to close.
			 */
			DCS_API void StopThread(Socket client, StopMode mode = StopMode::IMMEDIATE);

			/**
			 * \brief Sends data to the client.
			 * \todo Make SendData read a schedulable queue thus sending data indirectly via other threads.
			 * \param client Client Socket.
			 * \param data Text/binary data to send.
			 * \param size Size of the data sent.
			 */

			DCS_REGISTER_CALL
			DCS_API void SendData(Socket client, const unsigned char* data, DCS::i16 size);

			/**
			 * \example sockets/echo_server.cpp.
			 * An example showing how to implement a simple echo server.
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
		 * accepted struct (see Messaging::IAbstractMessage), which is responsible for knowing which operation to perform
		 * on the server-side.
		 *
		 * \todo Finish this documentation, use a custom msg read callback
		 */
		namespace Messaging
		{
			enum class DCS_API Operation
			{
				NO_OP,     ///< Ping the server only.
				REQUEST,   ///< Request something to server.
				SUB_EVT,   ///< Subscribe to a server-side value(s) change.
				UNSUB_EVT  ///< Unsubscribe from a previously subscribed event.
			};

			template<typename... Ts>
			struct DCS_API Command
			{
			private:
#pragma warning( push )
#pragma warning( disable : 4251 ) // Ignore the dll-interface warning, since this member will not be accessible
				inline static std::unordered_map<const char*, i16> map;
				std::tuple<Ts...> params;
#pragma warning( pop )

			public:
				static void RegisterCallableFunc();
			};

			/*template<class F, class... Args>
			class DCS_API IAbstractMessage
			{
			protected:
				IAbstractMessage(Operation op, Command<F, Args...> cmd) : c(cmd), o(op) {  };
				virtual ~IAbstractMessage() {  };
				decltype(auto) execute()
				{
					return c();
				}

			protected:
				Operation o;
				Command<F, Args...> c;
			};

			template<class F, class... Args>
			class DCS_API DefaultMessage : public IAbstractMessage<F, Args...>
			{
			public:
				DefaultMessage(Operation op, Command<F, Args...> cmd) : IAbstractMessage<F, Args...>(op, cmd) {  };

				decltype(auto) test()
				{
					return IAbstractMessage<F, Args...>::execute();
				}
			};*/
		}
	}
}
