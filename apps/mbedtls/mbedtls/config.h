#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

//Cleared entire config, no need for tls or dtls or ...
//Only using algorithm implementations like aes, md, ...

/* Save RAM at the expense of ROM */
#define MBEDTLS_AES_ROM_TABLES

#define MBEDTLS_SHA256_C
#define MBEDTLS_CCM_C
#define MBEDTLS_MD_C
#define MBEDTLS_CTR_DRBG_C

#define MBEDTLS_AES_C

#define MBEDTLS_AES_DECRYPT_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_SETKEY_ENC_ALT

#define MBEDTLS_CIPHER_C

#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY




#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */