#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

//Cleared entire config, no need for tls or dtls or ...
//Only using algorithm implementations like aes, md, ...

/* Save RAM at the expense of ROM */
#define MBEDTLS_AES_ROM_TABLES


#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */