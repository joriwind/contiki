#include "obj-sec.h"
#include "cose.h"
#include "cn-cbor.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define OBJ_SEC_KEYSIZE 128

const byte key[OBJ_SEC_KEYSIZE];

void objsec_init(){
    memset(key, 0, OBJ_SEC_KEYSIZE);
}

void objsec_set_key(uint8_t *k){
    memcpy(key, k, OBJ_SEC_KEYSIZE);
}

size_t encrypt(uint8_t *buffer, uint16_t bufferSz, const uint8_t *message, size_t len) {
  //HCOSE_ENCRYPT  COSE_Encrypt_Init(COSE_INIT_FLAGS flags, CBOR_CONTEXT_COMMA cose_errback * perr);
  //bool COSE_Encrypt_SetContent(HCOSE_ENCRYPT cose, const byte * rgbContent, size_t cbContent, cose_errback * errp);
  //bool COSE_Encrypt_encrypt(HCOSE_ENCRYPT cose, const byte * pbKey, size_t cbKey, cose_errback * perror);
  cose_errback err;
  HCOSE_ENCRYPT objcose;
  

  /* INIT FLAGS
  COSE_INIT_FLAGS_NONE=0,
	COSE_INIT_FLAGS_DETACHED_CONTENT=1,
	COSE_INIT_FLAGS_NO_CBOR_TAG=2,
	COSE_INIT_FLAGS_ZERO_FORM=4
  */
  objcose = COSE_Encrypt_Init(COSE_INIT_FLAGS_ZERO_FORM, &context,&err);

  if( objcose == NULL ) {
    PRINTF("Error in init cose: %i\n", err.err);
    return -1;
  }

  if( !COSE_Encrypt_SetContent(objcose, message, len, &err)){
    PRINTF("Error in set content cose: %i\n",err.err);
    return -1;
  }

  if( !COSE_Encrypt_encrypt(objcose, key, OBJ_SEC_KEYSIZE, &err)){
    PRINTF("Error in encrypt cose: %i\n", err.err);
    return -1;
  }

  return COSE_Encode(objcose, buffer, 0, bufferSz);
  
}