#include "../../DCS_Core/include/internal.h"
#include "../include/DCS_Assert.h"

int main()
{
    using namespace DCS::Auth;

    DCS::u8 hs[DCS_SHA256_DIGEST_LENGTH];
    char hs_str[DCS_SHA256_DIGEST_LENGTH * 2];

    InitCryptoRand();

    SHA256Str("hjf7$", hs);
    HexStringifyBytes(hs_str, hs, DCS_SHA256_DIGEST_LENGTH);
    printf("hjf7$ - sha256: %s\n", hs_str);

    DCS::u8 iv[16];
    GenerateRandSafeIV128(iv);

    DCS::u8 original[32];
    DCS::u8 got_original[32];
    DCS::u8 em[32];
    DCS::u8 tag[16];

    const char* odata = "This is the original data!32Sz!";

    memcpy(original, odata, 32);
    EncryptAES256(original, 32, nullptr, 0, hs, iv, em, tag);

    printf("Original: ");
    for(int i = 0; i < 32; i++) printf("%02x", original[i]);
    printf("\nEncrypted (AES256 GCM): ");
    for(int i = 0; i < 32; i++) printf("%02x", em[i]);
    printf("\nTag: ");
    for(int i = 0; i < 16; i++) printf("%02x", tag[i]);
    printf("\n");

    //GenerateRandSafeIV128(iv);
    DecryptAES256(em, 32, nullptr, 0, hs, iv, got_original, tag);

    printf("Decrypted original: ");
    for(int i = 0; i < 16; i++) printf("%02x", got_original[i]);
    printf("\n");

    printf("Converted output: %s\n", (char*)got_original);
    
    return 0;
}