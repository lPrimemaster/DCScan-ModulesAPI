#include "../include/internal.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static bool cRandInit = false;

void DCS::Auth::InitCryptoRand()
{
    // HACK : Care for RAND_poll, might crash on windows
    RAND_poll();
    cRandInit = true;
}

// Used to SHA256 passwords before storing
void DCS::Auth::GenerateSalt(DCS::u8 salt[8])
{
    if(salt == nullptr)
    {
        LOG_ERROR("DCS::Auth::GenerateSalt requires a non null 64-bit pointer.");
        return;
    }

    if(cRandInit)
        RAND_bytes(salt, 8 * sizeof(u8));
    else
        LOG_ERROR("Failed to generate salt. Was InitCryptoRand called?");
}

void DCS::Auth::GenerateRandSafeIV128(DCS::u8 iv[16])
{
    if(iv == nullptr)
    {
        LOG_ERROR("DCS::Auth::GenerateRandSafeIV128 requires a non null 128-bit pointer.");
        return;
    }

    if(cRandInit)
        RAND_bytes(iv, 16 * sizeof(u8));
    else
        LOG_ERROR("Failed to generate IV128. Was InitCryptoRand called?");
}

void DCS::Auth::SHA256Str(char *string, DCS::u8 hash[SHA256_DIGEST_LENGTH])
{
    if(hash == nullptr)
    {
        LOG_ERROR("DCS::Auth::SHA256Str requires a non null 256-bit pointer.");
        return;
    }

    if(string == nullptr || strlen(string) == 0)
    {
        LOG_ERROR("DCS::Auth::SHA256Str requires a non null, non zero sized string.");
        return;
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
}

void DCS::Auth::HexStringifyBytes(char* out, DCS::u8* hash, DCS::u64 size)
{
    if(hash == nullptr)
    {
        LOG_ERROR("DCS::Auth::HexStringifyBytes requires a non null, non empty pointer.");
        return;
    }
    if(out == nullptr)
    {
        LOG_ERROR("DCS::Auth::HexStringifyBytes requires a non null output string.");
        return;
    }

    for(int i = 0; i < size; i++)
    {
        sprintf(&out[i * 2], "%02x", hash[i]);
    }
}
