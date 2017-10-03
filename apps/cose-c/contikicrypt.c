#include "cose.h"
#include "configure.h"
#include "cose_int.h"
#include "crypto.h"

//#include <memory.h>

#ifdef CONTIKI

#include <random.h>
#include <stdio.h>
#include "ccm-star.h"
#include "cc2420.h"

static const unsigned short seed = 25647;


uint16_t i;
#define PRINTBUF(buffer, begin, end)  printf("Buffer:{"); for (i = begin; i < end; i++){ \
	printf(" %x", buffer[i]);  \
  } \
  printf("}\n");



bool AES_CCM_Decrypt(COSE_Enveloped * pcose, int TSize, int LSize, const byte * pbKey, size_t cbKey, const byte * pbCrypto, size_t cbCrypto, const byte * pbAuthData, size_t cbAuthData, cose_errback * perr)
{
	uint16_t NSize = 15 - (LSize / 8); //Conversion?
	TSize /= 8; // Comes in in bits not bytes.
    uint8_t check_tag[15] = {0};
	const cn_cbor * pIV = NULL;
    uint16_t cbOut;
    int i;
    //  Setup the IV/Nonce and put it into the message
	pIV = _COSE_map_get_int(&pcose->m_message, COSE_Header_IV, COSE_BOTH, NULL);
	if ((pIV == NULL) || (pIV->type!= CN_CBOR_BYTES)) {
		if (perr != NULL) perr->err = COSE_ERR_INVALID_PARAMETER;
        return false;
	}
	printf("IV: ");
	PRINTBUF((uint8_t *)pIV->v.str, 0, pIV->length);

    CHECK_CONDITION(pIV->length == NSize, COSE_ERR_INVALID_PARAMETER);
	//memcpy(rgbIV, pIV->v.str, pIV->length);

    CHECK_CONDITION(cbKey == 128/8, COSE_ERR_CRYPTO_FAIL);
    set_key1((uint8_t *)pbKey);
	en_key(1);
    
    cbOut = (uint16_t)  cbCrypto - TSize;
    //Decryption
	CCM_STAR.ctr((uint8_t *)pbCrypto, cbOut, (uint8_t *)pIV->v.str);
	
    CCM_STAR.mic((uint8_t *)pbCrypto, cbOut, (uint8_t *)pIV->v.str, pbAuthData,  cbAuthData, check_tag, TSize);
	

    if(memcmp(check_tag, &pbCrypto[cbOut], TSize) != 0){
		printf("Tag mismatch\n");
		printf("ChckT: ");
		PRINTBUF(check_tag, 0, TSize);
		printf("Tag: ");
		PRINTBUF(pbCrypto, cbOut, cbOut + TSize);
		perr->err = COSE_ERR_CRYPTO_FAIL;
		goto errorReturn;
	}


	printf("Restore LLSEC key\n");
	en_key(0);


	pcose->pbContent = pbCrypto;
	pcose->cbContent = cbOut;
	return true;

errorReturn:
	printf("Restore LLSEC key\n");
	en_key(0);
    return false;
}


bool AES_CCM_Encrypt(COSE_Enveloped * pcose, int TSize, int LSize, const byte * pbKey, size_t cbKey, const byte * pbAuthData, size_t cbAuthData, cose_errback * perr)
{
	uint16_t cbOut;
	uint8_t NSize = 15 - (LSize/8);
	const cn_cbor * cbor_iv = NULL;
    uint8_t iv[16];
    TSize /= 8; // Comes in in bits not bytes.
	cn_cbor * cnTmp = NULL;
	cn_cbor * cnNonce = NULL;
    
    cbOut = pcose->cbContent;
	uint8_t ciphertxt[cbOut + TSize];
	memset(ciphertxt, 0, cbOut + TSize);

	byte * nonce = NULL;

    cn_cbor_errback cn_err;

#ifdef USE_CBOR_CONTEXT
	cn_cbor_context * context = &pcose->m_message.m_allocContext;
#endif

	//  Setup the IV/Nonce and put it into the message
	cbor_iv = _COSE_map_get_int(&pcose->m_message, COSE_Header_IV, COSE_BOTH, perr);
	if (cbor_iv == NULL) {
		printf("IV is zero\n");
		nonce = COSE_CALLOC(NSize, 1, context);
		CHECK_CONDITION(nonce != NULL, COSE_ERR_OUT_OF_MEMORY);
		CHECK_CONDITION(LSize == 16, COSE_ERR_CRYPTO_FAIL);
		
		CHECK_CONDITION(iv != NULL, COSE_ERR_OUT_OF_MEMORY);
		printf("Nonce size: %i\n", NSize);
		rand_bytes(nonce, NSize);
		printf("Nonce: ");
		PRINTBUF(nonce, 0, NSize);
		memcpy(iv, nonce, NSize);

		cnNonce = cn_cbor_data_create(iv, NSize, CBOR_CONTEXT_PARAM_COMMA &cn_err);
		CHECK_CONDITION_CBOR(cnNonce != NULL, cn_err);
		printf("IV: ");
		PRINTBUF(cnNonce->v.str, 0, cnNonce->length);

		if (!_COSE_map_put(&pcose->m_message, COSE_Header_IV, cnNonce, COSE_UNPROTECT_ONLY, perr)) goto errorReturn;
		cnNonce = NULL;
	}
	else {
		printf("IV is not zero\n");
		CHECK_CONDITION(cbor_iv->type == CN_CBOR_BYTES, COSE_ERR_INVALID_PARAMETER);
		CHECK_CONDITION(cbor_iv->length == LSize, COSE_ERR_INVALID_PARAMETER);
		memcpy(iv, cbor_iv->v.str, cbor_iv->length);
	}

	printf("IV/Nonce setup complete\n");

	//Set the key to use
    CHECK_CONDITION(cbKey == 128/8, COSE_ERR_CRYPTO_FAIL);
    set_key1((uint8_t *)pbKey);
	en_key(1);
	
    memcpy(ciphertxt, pcose->pbContent, cbOut);
	CCM_STAR.mic(ciphertxt, cbOut, iv, pbAuthData, cbAuthData, &ciphertxt[cbOut], TSize);
	
	CCM_STAR.ctr(ciphertxt, cbOut, iv);
	

	printf("Tag calculated\n");
	printf("Tag size: %i\n", TSize);
	cnTmp = cn_cbor_data_create(ciphertxt, cbOut + TSize, CBOR_CONTEXT_PARAM_COMMA NULL);
	CHECK_CONDITION(cnTmp != NULL, COSE_ERR_CBOR);

	CHECK_CONDITION(_COSE_array_replace(&pcose->m_message, cnTmp, INDEX_BODY, CBOR_CONTEXT_PARAM_COMMA NULL), COSE_ERR_CBOR);
	cnTmp = NULL;

	
	printf("Restore LLSEC key\n");
	en_key(0);
	
	return true;

errorReturn:
	printf("Restore LLSEC key\n");
	en_key(0);
	return false;
}


void rand_bytes(byte* pb, size_t cb){
	//CONTIKI way
	//random_init(seed);
	int i;
	for( i = 0; i < cb; i++){
		pb[i] = (byte)random_rand();
	}


}

#endif

