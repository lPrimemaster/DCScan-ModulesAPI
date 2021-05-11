#pragma once
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <functional>
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
	namespace DB // TODO : Document & Add user login statistics / trace
	{
#pragma pack( push )
		// NOTE : Using a salt is nice but requires to send the plain text password which is okay but such security is not needed.
		// Just ensure the user has a strong password.
		struct User
		{
			char u[32]; ///< Holds the username.
			u8   p[32]; ///< Holds the sha-256 of the password.
		};
#pragma pack( pop )

		DCS_INTERNAL_TEST void LoadDefaultDB();

		DCS_INTERNAL_TEST void CloseDB();

		DCS_INTERNAL_TEST void LoadUsers();

		DCS_INTERNAL_TEST void AddUser(const char* username, const char* password);

		DCS_INTERNAL_TEST void RemoveUserByUsername(const char* username);

		DCS_INTERNAL_TEST u64  FindUserByUsername(const char* username);

		DCS_INTERNAL_TEST User GetUser(const char* username);

		DCS_INTERNAL_TEST const User* GetAllUsers();

		DCS_INTERNAL_TEST u64  GetUserCount();
	}

	namespace Auth // TODO : Document
	{
		DCS_INTERNAL_TEST void InitCryptoRand();

		DCS_INTERNAL_TEST void GenerateSalt(DCS::u8 salt[8]);

		DCS_INTERNAL_TEST void GenerateRandSafeIV128(DCS::u8 iv[16]);

		DCS_INTERNAL_TEST void SHA256Str(const char* string, DCS::u8 hash[DCS_SHA256_DIGEST_LENGTH]);

		DCS_INTERNAL_TEST void HexStringifyBytes(char* out, DCS::u8* hash, DCS::u64 size);

		DCS_INTERNAL_TEST void EncryptAES256(DCS::u8* to_encrypt, int to_encrypt_size, 
											 DCS::u8* aad, int aad_size, DCS::u8* key, 
											 DCS::u8* iv, DCS::u8* encrypted_out, DCS::u8* tag);

		DCS_INTERNAL_TEST int  DecryptAES256(DCS::u8* cipher, int cipher_size, 
											 DCS::u8* aad, int aad_size, DCS::u8* key, 
											 DCS::u8* iv, DCS::u8* plain_out, DCS::u8* tag);
	}
}