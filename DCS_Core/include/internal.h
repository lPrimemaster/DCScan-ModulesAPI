#pragma once
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <type_traits>
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

#define DCS_SHA256_DIGEST_LENGTH 32

/**
 * @file
 * 
 * \internal
 * 
 * \brief Internal file exposing multiple functionalities (see specific documentation of members in this file.
 *
 * \author Cesar Godinho
 *
 * \version 1.0
 *
 * \date $Date: 2020/11/15$
 */
namespace DCS
{
	namespace Threading
	{
		/**
		 * \internal
		 * \brief A struct that holds data for a worker thread pool.
		 */
		struct DCS_INTERNAL_TEST TPool
		{
			std::vector<std::thread> workers;
			std::mutex lock;
			std::condition_variable signal;
			std::array<std::atomic_int, 16> flags;
		};

		/**
		 * \internal
		 * \brief Creates a fixed size thread pool.
		 * 
		 * All the passed functions are immutable in this mode. Making it perfect for long term 
		 * jobs with repeating tasks.
		 */
		DCS_INTERNAL_TEST TPool* CreatePersistentPool(u16 size, std::vector<std::function<void(std::mutex*, std::condition_variable*, std::array<std::atomic_int, 16>*)>> workers);

		/**
		 * \internal
		 * \brief Joins all threads inside a thread pool.
		 * 
		 * Refer to std::thread::join().
		 */
		DCS_INTERNAL_TEST void JoinPool(TPool* pool);

		/**
		 * \internal
		 * \brief Get the worker size of a TPool.
		 */
		DCS_INTERNAL_TEST u64 GetPoolWorkCount(TPool* pool);

		/**
		 * \internal
		 * \brief Deletes all data and workers associated with
		 * a certain TPool object.
		 */
		DCS_INTERNAL_TEST void DestroyPool(TPool* pool);
	}

	// TODO : Add a value database for any persistent settings (offsets)
	// C1 Offset to 180 -> -120.637650 deg
	// C2 Offset to 180 ->  -90.526640 deg
	namespace RDB
	{
		/**
		 * \internal
		 * \brief Enumerates the database currently supported data types.
		 */
		enum class EntryType
		{
			TEXT,
			INT,
			REAL,
			DATE,
			BLOB
		};

#define FOREACH_OPTYPE(LOGOP) \
	LOGOP(VAR_CREATE)		  \
	LOGOP(VAR_UPDATE)		  \
	LOGOP(VAR_DELETE)		  \
	LOGOP(USR_CREATE)		  \
	LOGOP(USR_DELETE)		  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
		enum class LogOperation
		{
			FOREACH_OPTYPE(GENERATE_ENUM)
		};

		static const char* LogOperationNames[] = 
		{
			FOREACH_OPTYPE(GENERATE_STRING)
		};
#undef FOREACH_OPTYPE
#undef GENERATE_ENUM
#undef GENERATE_STRING

		/**
		 * \internal
		 * \brief Open the realtime database.
		 */
		DCS_INTERNAL_TEST void OpenDatabase();

		/**
		 * \internal
		 * \brief Close the realtime database.
		 */
		DCS_INTERNAL_TEST void CloseDatabase();

		/**
		 * \internal
		 * \brief Query database entries for a given table and statements.
		 */
		DCS_INTERNAL_TEST void QueryEntries(const std::string& table,
											const std::string& column,
											const std::string& statement,
											EntryType type,
											void* data);

		/**
		 * \internal
		 * \brief Write a variable to the database.
		 */
		DCS_INTERNAL_TEST void WriteVariable(const char* name, const char* value, const char* descriptor = nullptr);
		
		/**
		 * \internal
		 * \brief Write a variable to the database.
		 */
		DCS_INTERNAL_TEST void WriteVariable(const char* name, f64 value, const char* descriptor = nullptr);

		/**
		 * \internal
		 * \brief Write a variable to the database (as system).
		 */
		DCS_INTERNAL_TEST void WriteVariableSys(const char* name, const char* value, const char* descriptor = nullptr);
		
		/**
		 * \internal
		 * \brief Write a variable to the database (as system).
		 */
		DCS_INTERNAL_TEST void WriteVariableSys(const char* name, f64 value, const char* descriptor = nullptr);

		/**
		 * \internal
		 * \brief Read a variable from the database.
		 */
		DCS_INTERNAL_TEST std::string ReadVariable(const char* name);
		
		/**
		 * \internal
		 * \brief Get a variable's description from the database.
		 */
		DCS_INTERNAL_TEST std::string GetVariableDescriptor(const char* name);

		/**
		 * \internal
		 * \brief Delete a variable from the database.
		 */
		DCS_INTERNAL_TEST void DeleteVariable(const char* name);
		
		/**
		 * \internal
		 * \brief Read a variable from the database (as system).
		 */
		DCS_INTERNAL_TEST void DeleteVariableSys(const char* name);

		/**
		 * \internal
		 * \brief Logs database events.
		 */
		DCS_INTERNAL_TEST void LogEventUser(LogOperation op, const char* what);
		
		/**
		 * \internal
		 * \brief Logs database events (as system).
		 */
		DCS_INTERNAL_TEST void LogEventSystem(LogOperation op, const char* what);

		/**
		 * \internal
		 * \brief Creates default Database tables.
		 */
		DCS_INTERNAL_TEST void CreateTables();

		/**
		 * \internal
		 * \brief Structure that holds everything thats is related to a single user.
		 */
		struct User
		{
			char u[32]; ///< Holds the username.
			u8   p[32]; ///< Holds the sha-256 of the password.
		};

		/**
		 * \internal
		 * \brief Adds a user to the database.
		 */
		DCS_INTERNAL_TEST void AddUser(const char* username, const char* password);

		/**
		 * \internal
		 * \brief Removes an existing user from the database.
		 */
		DCS_INTERNAL_TEST void RemoveUser(const char* username);

		/**
		 * \internal
		 * \brief Finds an existing user from the database.
		 * \return User struct of the found user.
		 */
		DCS_INTERNAL_TEST User GetUser(const char* username);

		/**
		 * \internal
		 * \brief Gets a read only copy of the database users.
		 * \return Array of all the User structs in the database.
		 */
		DCS_INTERNAL_TEST const std::vector<User> GetAllUsers();

		/**
		 * \internal
		 * \brief Sets the current authenticated user server-side.
		 * This function does not grant privileges to the user.
		 * \param user The user to set as authenticated.
		 */
		DCS_INTERNAL_TEST void SetAuthenticatedUser(const User* user);
	}

	namespace Auth
	{
		/**
		 * \internal
		 * \brief Initializes a safe crypto random generator.
		 */
		DCS_INTERNAL_TEST void InitCryptoRand();

		/**
		 * \internal
		 * \brief Generates a cryptographically secure random 64-bit value.
		 */
		DCS_INTERNAL_TEST void GenerateSalt(DCS::u8 salt[8]);

		/**
		 * \internal
		 * \brief Generates a cryptographically secure random 128-bit value.
		 */
		DCS_INTERNAL_TEST void GenerateRandSafeIV128(DCS::u8 iv[16]);

		/**
		 * \internal
		 * \brief Generates a string's SHA-256 hash.
		 */
		DCS_INTERNAL_TEST void SHA256Str(const char* string, DCS::u8 hash[DCS_SHA256_DIGEST_LENGTH]);

		/**
		 * \internal
		 * \brief Returns bytes as a string (for debugging purposes only).
		 */
		DCS_INTERNAL_TEST void HexStringifyBytes(char* out, DCS::u8* hash, DCS::u64 size);

		/**
		 * \internal
		 * \brief Encrypts bytes with the AES-256 (GCM) cypher using a key and iv.
		 */
		DCS_INTERNAL_TEST void EncryptAES256(DCS::u8* to_encrypt, int to_encrypt_size, 
											 DCS::u8* aad, int aad_size, DCS::u8* key, 
											 DCS::u8* iv, DCS::u8* encrypted_out, DCS::u8* tag);

		/**
		 * \internal
		 * \brief Decrypts bytes with the AES-256 (GCM) cypher using a key and iv.
		 */
		DCS_INTERNAL_TEST int  DecryptAES256(DCS::u8* cipher, int cipher_size, 
											 DCS::u8* aad, int aad_size, DCS::u8* key, 
											 DCS::u8* iv, DCS::u8* plain_out, DCS::u8* tag);
	}

	namespace Core
	{
		class DCS_INTERNAL_TEST PID
		{
		public:
			PID(f64 min, f64 max, f64 Kp, f64 Kd, f64 Ki);

			void setTargetAndBias(f64 target, f64 bias);

			f64 calculate(f64 value);

			f64 calculate(f64 value, f64 dt);

		private:
			f64 min;
			f64 max;
			f64 Kp;
			f64 Kd;
			f64 Ki;

			f64 dt;

			f64 target;
			f64 bias;

			f64 le;
			f64 integral;

			std::chrono::steady_clock::time_point last_point;
		};
	}
}
