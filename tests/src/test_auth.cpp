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

    DCS::u8 salt[8];
    GenerateSalt(salt);
    printf("Random salt: ");
    for(int i = 0; i < 8; i++) printf("%02x", salt[i]);
    printf("\n");

    DCS::u8 iv[16];
    GenerateRandSafeIV128(iv);
    printf("Random IV128: ");
    for(int i = 0; i < 16; i++) printf("%02x", iv[i]);
    printf("\n");
    
    return 0;
}