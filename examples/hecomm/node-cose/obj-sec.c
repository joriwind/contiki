#include "obj-sec.h"
#include "cose.h"
#include "cn-cbor.h"
#include "cose_int.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define OBJ_SEC_KEYSIZE 16



#define INIT_KEY {0x1, 0x1, 0x1, 0x1, \
                  0x1, 0x1, 0x1, 0x1, \
                  0x1, 0x1, 0x1, 0x1, \
                  0x1, 0x1, 0x1, 0x1};

static byte key[OBJ_SEC_KEYSIZE] = INIT_KEY;
static cn_cbor * algorithm;
static uint8_t keyset = 0;

void objsec_init(){
    //memcpy(key, INIT_KEY, OBJ_SEC_KEYSIZE);
    cn_cbor_errback err;
    keyset = 0;
    algorithm = cn_cbor_int_create(COSE_Algorithm_AES_CCM_16_64_128,&context, &err);
    if(algorithm == NULL){
      PRINTF("Could not allocate algorithm object: %u\n", err.err);
    }
}

uint8_t objsec_key_set(){
  return keyset;
}

void objsec_set_key(uint8_t *k){
    memcpy(key, k, OBJ_SEC_KEYSIZE);
    keyset = 1;
}

size_t encrypt(uint8_t *buffer, uint16_t bufferSz, const uint8_t *message, size_t len) {
  //HCOSE_ENCRYPT  COSE_Encrypt_Init(COSE_INIT_FLAGS flags, CBOR_CONTEXT_COMMA cose_errback * perr);
  //bool COSE_Encrypt_SetContent(HCOSE_ENCRYPT cose, const byte * rgbContent, size_t cbContent, cose_errback * errp);
  //bool COSE_Encrypt_encrypt(HCOSE_ENCRYPT cose, const byte * pbKey, size_t cbKey, cose_errback * perror);
  cose_errback err;
  HCOSE_ENCRYPT objcose;
  uint8_t temp[1] = {""};
  size_t temp_len = 0;
  

  /* INIT FLAGS
  COSE_INIT_FLAGS_NONE=0,
	COSE_INIT_FLAGS_DETACHED_CONTENT=1,
	COSE_INIT_FLAGS_NO_CBOR_TAG=2,
	COSE_INIT_FLAGS_ZERO_FORM=4
  */
  objcose = COSE_Encrypt_Init(COSE_INIT_FLAGS_NONE, &context,&err);

  if( objcose == NULL ) {
    PRINTF("Error in init cose: %i\n", err.err);
    return -1;
  }

  if( !COSE_Encrypt_SetContent(objcose, message, len, &err)){
    PRINTF("Error in set content cose: %i\n",err.err);
    goto errorReturn;
  }

  //if(!COSE_Encrypt_map_put_int(objcose, COSE_Header_Algorithm, algorithm, COSE_DONT_SEND, &err)){
  if(!COSE_Encrypt_map_put_int(objcose, COSE_Header_Algorithm, algorithm, COSE_UNPROTECT_ONLY, &err)){
    PRINTF("Error in setting algorithm %i\n", err.err);
    goto errorReturn;
  }

  //Setting AAD
  if(!COSE_Encrypt_SetExternal(objcose, temp, temp_len, &err)){
    PRINTF("Error in setting AAD %i\n", err.err);
    goto errorReturn;
  }

  if( !COSE_Encrypt_encrypt(objcose, key, OBJ_SEC_KEYSIZE, &err)){
    PRINTF("Error in encrypt cose: %i\n", err.err);
    goto errorReturn;
  }


  size_t size = COSE_Encode((HCOSE) objcose, buffer, 0, bufferSz);
  printf("Buffer filled\n");
  COSE_Encrypt_Free(objcose);
  clear_memory(&context);
  printf("Objcose released!\n");
  return size;

errorReturn:
  if (objcose != NULL) {
    printf("Releasing objcose\n");
    COSE_Encrypt_Free(objcose);
    clear_memory(&context);
    printf("Objcose released!\n");
  }
  return -1;
  
}

size_t decrypt(const uint8_t *message, size_t len, uint8_t *buffer, size_t bufferSz) {
  //HCOSE_ENCRYPT  COSE_Encrypt_Init(COSE_INIT_FLAGS flags, CBOR_CONTEXT_COMMA cose_errback * perr);
  //bool COSE_Encrypt_SetContent(HCOSE_ENCRYPT cose, const byte * rgbContent, size_t cbContent, cose_errback * errp);
  //bool COSE_Encrypt_encrypt(HCOSE_ENCRYPT cose, const byte * pbKey, size_t cbKey, cose_errback * perror);
  cose_errback err;
  HCOSE_ENCRYPT objcose;

  int ptype;
  
  objcose = (HCOSE_ENCRYPT)COSE_Decode(message, len, &ptype, COSE_encrypt_object, &context,&err);
  if (objcose == NULL){
    printf("Unable to decode cose object: %i\n", err.err);
    return - 1;
  }

  cn_cbor * alg = COSE_Encrypt_map_get_int(objcose, COSE_Header_Algorithm, COSE_BOTH, &err);
  if (alg == NULL){
    printf("Unable to get used algorithm: %i\n", err.err);
    goto errorReturn;
  }

  if ((alg->type != CN_CBOR_INT) && (alg->type != CN_CBOR_UINT)) return true;
	if (alg->v.sint != COSE_Algorithm_AES_CCM_16_64_128){
    printf("Algorithm not supported: %i\n", (int)alg->v.sint);
    goto errorReturn;
  }
  
  if(!COSE_Encrypt_decrypt(objcose, key, OBJ_SEC_KEYSIZE, &err)){
    printf("Unable to decrypt COSE object: %i\n", err.err);
    goto errorReturn;
  }

  COSE_Encrypt * pcose = (COSE_Encrypt *)objcose;
  if(pcose->cbContent > bufferSz){
    printf("Buffer too small!\n");
    return -1;
  }
  memcpy(buffer, pcose->pbContent, pcose->cbContent);
  
  return pcose->cbContent;

errorReturn:
  if (objcose != NULL) {
    printf("Releasing objcose\n");
    COSE_Encrypt_Free(objcose);
    clear_memory(&context);
    printf("Objcose released!\n");
  }
  return -1;
  
}