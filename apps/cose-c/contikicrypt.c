#include "cose.h"
#include "configure.h"
#include "cose_int.h"
#include "crypto.h"

//#include <memory.h>

#ifdef CONTIKI

#include <random.h>
#include <stdio.h>
#include "ccm-star.h"

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
	return true;

errorReturn:
    return false;
}


bool AES_CCM_Encrypt(COSE_Enveloped * pcose, int TSize, int LSize, const byte * pbKey, size_t cbKey, const byte * pbAuthData, size_t cbAuthData, cose_errback * perr)
{
	int cbOut;
	LSize = 15 - (LSize/8);
	const cn_cbor * cbor_iv = NULL;
    uint8_t iv[16];
    TSize /= 8; // Comes in in bits not bytes.
	cn_cbor * cnTmp = NULL;
    cn_cbor * cnIV = NULL;
    
    cbOut = pcose->cbContent;
    uint8_t ciphertxt[cbOut + TSize];

    cn_cbor_errback cn_err;

#ifdef USE_CBOR_CONTEXT
#warning AES_ENCYRPT CONTEXT ENABLED!!
	cn_cbor_context * context = &pcose->m_message.m_allocContext;
#endif

	//  Setup the IV/Nonce and put it into the message
	cbor_iv = _COSE_map_get_int(&pcose->m_message, COSE_Header_IV, COSE_BOTH, perr);
	if (cbor_iv == NULL) {
        CHECK_CONDITION(LSize == 16, COSE_ERR_CRYPTO_FAIL);
		CHECK_CONDITION(iv != NULL, COSE_ERR_OUT_OF_MEMORY);
		rand_bytes(iv, LSize);

		cnIV = cn_cbor_data_create(iv, LSize, CBOR_CONTEXT_PARAM_COMMA &cn_err);
		CHECK_CONDITION_CBOR(cnIV != NULL, cn_err);

		if (!_COSE_map_put(&pcose->m_message, COSE_Header_IV, cnIV, COSE_UNPROTECT_ONLY, perr)) goto errorReturn;
		cnIV = NULL;
	}
	else {
		CHECK_CONDITION(cbor_iv->type == CN_CBOR_BYTES, COSE_ERR_INVALID_PARAMETER);
		CHECK_CONDITION(cbor_iv->length == LSize, COSE_ERR_INVALID_PARAMETER);
		memcpy(iv, cbor_iv->v.str, cbor_iv->length);
	}

    CHECK_CONDITION(cbKey == 128, COSE_ERR_CRYPTO_FAIL);
    CCM_STAR.set_key(pbKey);

    //Copy over message to result buffer
    memcpy(ciphertxt, pcose->pbContent, pcose->cbContent);
    CCM_STAR.ctr(ciphertxt, pcose->cbContent, iv);
    CCM_STAR.mic(pcose->pbContent, pcose->cbContent, iv, pbAuthData, cbAuthData, &ciphertxt[cbOut], TSize);

	cnTmp = cn_cbor_data_create(ciphertxt, (int)pcose->cbContent + TSize, CBOR_CONTEXT_PARAM_COMMA NULL);
	CHECK_CONDITION(cnTmp != NULL, COSE_ERR_CBOR);

	CHECK_CONDITION(_COSE_array_replace(&pcose->m_message, cnTmp, INDEX_BODY, CBOR_CONTEXT_PARAM_COMMA NULL), COSE_ERR_CBOR);
	cnTmp = NULL;

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

