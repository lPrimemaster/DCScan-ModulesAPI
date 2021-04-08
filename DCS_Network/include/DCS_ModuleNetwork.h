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
				WAIT	   ///< Stop server after waiting for the client to disconnect (blocking the calling thread).
			};

			/**
			 * \brief Create server listening port.
			 * \param port Port to bind the server.
			 */
			DCS_API Socket Create(i32 port);

			/**
			 * \brief Wait for any client to connect to the server connection Socket.
			 * \param server Created server socket to wait on.
			 */
			DCS_API void WaitForConnections(Socket server);

			/**
			 * \brief Closes the server listening socket, no longer accepting new connections.
			 * \param server Created server socket to perform the operation.
			 */
			DCS_API void StopListening(Socket server);

			/**
			 * \brief Starts the server thread on a established connection.
			 * \param client The connection between server and client returned by WaitForConnection(Socket).
			 * \return Thread init success.
			 */
			DCS_API bool StartThread(Socket client);

			/**
			 * \brief Stops the server thread on a established connection (Forces disconnect).
			 * \param client The connection between server and client that is currently active.
			 * \param mode How the server thread stops (view StopMode).
			 */
			DCS_API void StopThread(Socket client, StopMode mode = StopMode::IMMEDIATE);

			/**
			 * \brief Returns the currently server connected client
			 */
			DCS_API Socket GetConnectedClient();

			/**
			 * \brief Returns the server listen socket
			 */
			DCS_API Socket GetListenSocket();

			/**
			 * \brief Checks if the server is running.
			 */
			DCS_API bool IsRunning();

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
			 * \return Thread init success.
			 */
			DCS_API bool StartThread(Socket connection);

			/**
			 * \brief Stops the client thread on a established connection (Forces disconnect).
			 * \todo Stopping the client thread while data is being transfered loops inf on the client.
			 * \param connection The connection between server and client that is currently active.
			 */
			DCS_API void StopThread(Socket connection);

			/**
			 * \brief Gets the current connection latency in milliseconds.
			 * \todo This is not working. Re-enable the tcp keep alive to fix.
			 * Possible last check: 10 seconds ago
			 * Interval: [0, Inf[ ms
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
				NO_OP = 0,	   ///< Do nothing.
				REQUEST = 1,   ///< Request a function call to the server.
							   // 2 and 3 reserved
				RESPONSE = 4,  ///< Send back a response to the client.
							   // 5 and 6 reserved
				EVT_SUB = 7,   ///< Subscribe to a server-side event.
							   // 8 reserved
				EVT_UNSUB = 9, ///< Unsubscribe from a previously subscribed event.
				OP_ERROR,	   ///< Send an error to the client/server.
				CON_VALID,	   ///< Server connection validity message.
				DATA		   ///< Send or receive data only.
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
			DCS_API void SendAsync(Operation op, u8 *data, i32 size);

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
			DCS_API Registry::SVReturn SendSync(Operation op, u8 *data, i32 size);

			/**
			 * \brief A Sample Event possible to implement in the server/API.
			 * When called, returns the next number in the Fibbonacci sequence.
			 * \ingroup events
			 */
			DCS_REGISTER_EVENT
			DCS_API void FibSeqEvt();
		}
	}

	/**
	 * \brief %Command Line Interface (CLI) responsible for handling server-side console commands.
	 */
	namespace CLI
	{
		/**
		 * \brief Class responsible for keeping the state of a declared command to use in the server console.
		 */
		class Command
		{
		public:
			using MapType = std::map<std::string, Command>;
			using RunCB = std::function<void(bool*)>;

			/**
			 * \internal
			 * \brief This constructor is used to register commands only. Do not use its return.  
			 */
			Command(const char *DCL, const char *help = "", RunCB cb = nullptr)
			{
				cmd_name = DCL;
				help_string = help;
				to_run = cb;

				if (!IsCommand(cmd_name))
					cmd_reg.emplace(cmd_name, *this);
				else
					LOG_WARNING("Attempted to create command %s. But it was already registered. Ignoring...", DCL);
			}

			/**
			 * \internal
			 * \brief Gets a named Command.
			 */
			static Command *Get(std::string name)
			{
				MapType::iterator it = cmd_reg.find(name);

				if (it != cmd_reg.end())
					return &it->second;
				return nullptr;
			}

			/**
			 * \internal
			 * \brief Checks if name is a Command.
			 */
			static bool IsCommand(std::string name)
			{
				MapType::iterator it = cmd_reg.find(name);
				return it != cmd_reg.end();
			}

			/**
			 * \internal
			 * \brief Founds most similar Command existing to name.
			 * \todo Fix for nonsense commands being matched. Set a maximum for the Levenshtein Distance (8 maybe?).
			 */
			static Command *Closest(std::string name);

			/**
			 * \internal
			 * \brief Lists all the available, registered, commands.
			 */
			static std::vector<std::string> ListCommands()
			{
				std::vector<std::string> cmds;
				for (auto &p : cmd_reg)
				{
					cmds.push_back(p.second.cmd_name + " - " + p.second.help_string);
				}
				return cmds;
			}

			/**
			 * \internal
			 * \brief Get the name of the Command.
			 */
			inline const std::string getName() const
			{
				return cmd_name;
			}

			/**
			 * \internal
			 * \brief Run the Command callback task.
			 */
			inline void Run(bool* brk)
			{
				if (to_run != nullptr)
				{
					to_run(brk);
				}

				// Silently ignore
			}

		private:
			inline static MapType cmd_reg;

		private:
			std::string cmd_name;
			std::string help_string;

			RunCB to_run;
		};

		/**
		 * \brief Make the current thread wait for, and process, console commands.
		 */
		DCS_API void Spin();
	}
}

#endif _DCS_NETWORK_H
