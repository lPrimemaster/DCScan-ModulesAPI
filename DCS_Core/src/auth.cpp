#include "../include/internal.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static bool cRandInit = false;

void DCS::Auth::InitCryptoRand()
{
    // HACK : Care for RAND_poll, might crash on windows
    if(!cRandInit) RAND_poll();
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

void DCS::Auth::SHA256Str(const char *string, DCS::u8 hash[SHA256_DIGEST_LENGTH])
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

void DCS::Auth::EncryptAES256(u8* to_encrypt, int to_encrypt_size, u8* aad, int aad_size, u8* key, u8* iv, u8* encrypted_out, u8* tag)
{
    EVP_CIPHER_CTX* ctx;

    int len, cipher_text_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        LOG_ERROR("Failed to initialize a new openssl cipher context.");
        return;
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
    {
        LOG_ERROR("Failed to setup AES256 (GCM mode) as the openssl cipher type.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1)
    {
        LOG_ERROR("Failed to setup the cipher iv length to 128-bit.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    if(EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1)
    {
        LOG_ERROR("Failed to assign the cipher's iv and key.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    if(aad_size > 0 && aad != nullptr)
    {
        if(EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_size) != 1)
        {
            LOG_WARNING("Failed to assign the cipher's message AAD.");
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
    }
    else
    {
        LOG_DEBUG("EncryptAES256 running without AAD data in encryption mode.");
    }

    if(EVP_EncryptUpdate(ctx, encrypted_out, &len, to_encrypt, to_encrypt_size) != 1)
    {
        LOG_ERROR("Failed to encrypt the message with AES256 in GCM mode.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    cipher_text_len = len;

    if(EVP_EncryptFinal_ex(ctx, encrypted_out + len, &len) != 1)
    {
        LOG_ERROR("Failed to finalize the message encryption with AES256 in GCM mode.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    cipher_text_len += len;

    if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1)
    {
        LOG_ERROR("Failed to get the GCM mode tag.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    EVP_CIPHER_CTX_free(ctx);
}

int DCS::Auth::DecryptAES256(u8* cipher, int cipher_size, u8* aad, int aad_size, u8* key, u8* iv, u8* plain_out, u8* tag)
{
    EVP_CIPHER_CTX *ctx;
    int len, plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        LOG_ERROR("Failed to initialize a new openssl cipher context.");
        return -1;
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
    {
        LOG_ERROR("Failed to setup AES256 (GCM mode) as the openssl cipher type.");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1)
    {
        LOG_ERROR("Failed to setup the cipher iv length to 128-bit.");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if(EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1)
    {
        LOG_ERROR("Failed to assign the cipher's iv and key.");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if(aad_size > 0 && aad != nullptr)
    {
        if(EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_size) != 1)
        {
            LOG_WARNING("Failed to assign the cipher's message AAD.");
            EVP_CIPHER_CTX_free(ctx);
            return -1;
        }
    }
    else
    {
        LOG_DEBUG("DecryptAES256 running without AAD data in decryption mode.");
    }

    if(EVP_DecryptUpdate(ctx, plain_out, &len, cipher, cipher_size) != 1)
    {
        LOG_ERROR("Failed to decrypt the message with AES256 in GCM mode.");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    if(tag != nullptr)
    {
        if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag) != 1)
        {
            LOG_ERROR("Failed to set the GCM mode tag.");
            EVP_CIPHER_CTX_free(ctx);
            return -1;
        }
    }

    int ret = EVP_DecryptFinal_ex(ctx, plain_out + len, &len);

    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0)
    {
        plaintext_len += len;
        return plaintext_len;
    }
    else
    {
        LOG_ERROR("Failed to decrypt.");
        return -1;
    }
}