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
 * \brief Internal file eposing multiple functionalities (see specific documentation of members in this file.
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

	/**
	 * \internal 
	 * \brief Database holding usernames, passwords, permissions, etc.
	 * 
	 * The database files and functions are thread-safe.
	 */
	namespace DB // TODO : Add user login statistics / trace
	{
#pragma pack( push )
		// NOTE : Using a salt is nice but requires to send the plain text password which is okay but such security is not needed.
		// Just ensure the user has a strong password.

		/**
		 * \internal
		 * \brief Structure that holds everything thats is (or will be) related with a single user.
		 */
		struct User
		{
			char u[32]; ///< Holds the username.
			u8   p[32]; ///< Holds the sha-256 of the password.
		};
#pragma pack( pop )

		/**
		 * \internal
		 * \brief Opens the default database file in the disk.
		 */
		DCS_INTERNAL_TEST void LoadDefaultDB();

		/**
		 * \internal
		 * \brief Closes the default database file in the disk.
		 */
		DCS_INTERNAL_TEST void CloseDB();

		/**
		 * \internal
		 * \brief Loads all the users stored in the default database file handle to a memory location.
		 */
		DCS_INTERNAL_TEST void LoadUsers();

		/**
		 * \internal
		 * \brief Adds an user to the default database file handle and memory location.
		 */
		DCS_INTERNAL_TEST void AddUser(const char* username, const char* password);

		/**
		 * \internal
		 * \brief Removes an existing user from the default database file handle and memory location.
		 */
		DCS_INTERNAL_TEST void RemoveUserByUsername(const char* username);

		/**
		 * \internal
		 * \brief Finds an existing user index from the memory location.
		 * \return User index in the database.
		 */
		DCS_INTERNAL_TEST u64  FindUserByUsername(const char* username);

		/**
		 * \internal
		 * \brief Finds an existing user from the memory location.
		 * \return User struct of the found user.
		 */
		DCS_INTERNAL_TEST User GetUser(const char* username);

		/**
		 * \internal
		 * \brief Gets a read only copy of the database memory location.
		 * \return Array of all the User structs in the database.
		 */
		DCS_INTERNAL_TEST const User* GetAllUsers();

		/**
		 * \internal
		 * \brief Gets the user count in the database memory location.
		 * \return Array of all the User structs in the database.
		 */
		DCS_INTERNAL_TEST u64  GetUserCount();
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
