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



bool AES_CCM_Decrypt(COSE_Enveloped * pcose, int TSize, int LSize, const byte * pbKey, size_t cbKey, const byte * pbCrypto, size_t cbCrypto, const byte * pbAuthData, size_t cbAuthData, cose_errback * perr)
{
    LSize = 15 - (LSize / 8); //Conversion?
    TSize /= 8; // Comes in in bits not bytes.
    uint8_t check_tag[16];
	const cn_cbor * pIV = NULL;
    int cbOut;
    int diff;
    int i;
    //  Setup the IV/Nonce and put it into the message
	pIV = _COSE_map_get_int(&pcose->m_message, COSE_Header_IV, COSE_BOTH, NULL);
	if ((pIV == NULL) || (pIV->type!= CN_CBOR_BYTES)) {
		if (perr != NULL) perr->err = COSE_ERR_INVALID_PARAMETER;
        return false;
    }

    CHECK_CONDITION(pIV->length == LSize, COSE_ERR_INVALID_PARAMETER);
	//memcpy(rgbIV, pIV->v.str, pIV->length);

    CHECK_CONDITION(cbKey*8 == 128, COSE_ERR_CRYPTO_FAIL)
    CCM_STAR.set_key(pbKey);
    
    cbOut = (int)  cbCrypto - TSize;
    //mic check
    CCM_STAR.mic((uint8_t *)pbCrypto,  cbCrypto, (uint8_t *)pIV, pbAuthData,  cbAuthData, check_tag, TSize);
    
    /* Check tag in "constant-time" */
    for( diff = 0, i = 0; i < TSize; i++ )
        diff |= pbCrypto[i + cbOut] ^ check_tag[i];
    CHECK_CONDITION(diff != 0, COSE_ERR_CRYPTO_FAIL);
    
    //Decryption
	CCM_STAR.ctr((uint8_t *)pbCrypto, cbOut, (uint8_t *)pIV);


	pcose->pbContent = pbCrypto;
	pcose->cbContent = cbOut;
	return true;

errorReturn:
    return false;
}


bool AES_CCM_Encrypt(COSE_Enveloped * pcose, int TSize, int LSize, const byte * pbKey, size_t cbKey, const byte * pbAuthData, size_t cbAuthData, cose_errback * perr)
{
	int cbOut;
	uint8_t NSize = 15 - (LSize/8);
	const cn_cbor * cbor_iv = NULL;
    uint8_t iv[16];
    TSize /= 8; // Comes in in bits not bytes.
	cn_cbor * cnTmp = NULL;
	cn_cbor * cnNonce = NULL;
    
    cbOut = pcose->cbContent;
    uint8_t ciphertxt[cbOut + TSize];

	byte * nonce = NULL;

    cn_cbor_errback cn_err;

#ifdef USE_CBOR_CONTEXT
#warning AES_ENCYRPT CONTEXT ENABLED!!
	cn_cbor_context * context = &pcose->m_message.m_allocContext;
#endif

	//  Setup the IV/Nonce and put it into the message
	cbor_iv = _COSE_map_get_int(&pcose->m_message, COSE_Header_IV, COSE_BOTH, perr);
	if (cbor_iv == NULL) {
		printf("IV is zero\n");
		nonce = COSE_CALLOC(NSize, 1, context);
		CHECK_CONDITION(nonce != NULL, COSE_ERR_OUT_OF_MEMORY);
        CHECK_CONDITION(LSize == 16, COSE_ERR_CRYPTO_FAIL);
		printf("LSize correct\n");
		CHECK_CONDITION(iv != NULL, COSE_ERR_OUT_OF_MEMORY);
		printf("iv is not zero\n");
		rand_bytes(nonce, NSize);
		memcpy(iv, nonce, NSize);

		cnNonce = cn_cbor_data_create(nonce, NSize, CBOR_CONTEXT_PARAM_COMMA &cn_err);
		CHECK_CONDITION_CBOR(cnNonce != NULL, cn_err);
		printf("cnNonce is not zero\n");

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
    set_key1(pbKey);
	en_key(1);
	printf("Key set\n");

    //Copy over message to result buffer
    memcpy(ciphertxt, pcose->pbContent, pcose->cbContent);
    CCM_STAR.ctr(ciphertxt, pcose->cbContent, iv);
    CCM_STAR.mic(pcose->pbContent, pcose->cbContent, iv, pbAuthData, cbAuthData, &ciphertxt[cbOut], TSize);

	printf("mic calculated\n");

	cnTmp = cn_cbor_data_create(ciphertxt, (int)pcose->cbContent + TSize, CBOR_CONTEXT_PARAM_COMMA NULL);
	CHECK_CONDITION(cnTmp != NULL, COSE_ERR_CBOR);

	CHECK_CONDITION(_COSE_array_replace(&pcose->m_message, cnTmp, INDEX_BODY, CBOR_CONTEXT_PARAM_COMMA NULL), COSE_ERR_CBOR);
	cnTmp = NULL;

	#if (LLSEC802154_CONF_ENABLED == 1)
	//Restore the key of LLSEC!
	printf("Restore LLSEC key\n");
	en_key(0);
	/* uint8_t key[16] = NONCORESEC_CONF_KEY;
	//uint8_t key[16] = {0,0,0,0 , 0,0,0,0 , 0,0,0,0 , 0,0,0,0};
	CCM_STAR.set_key(key); */
	#endif

	return true;

errorReturn:
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

