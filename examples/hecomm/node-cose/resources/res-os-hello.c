/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "rest-engine.h"
#include "cose.h"
#include "cn-cbor.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

//typedef unsigned char byte;
int encrypt(uint8_t *buffer, const uint8_t *message, size_t len, const byte *key, size_t szKey, uint16_t prefsz);

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

#include "lib/mmem.h"
static struct mmem mmem;
static cn_cbor_context ctx = (cn_cbor_context){.calloc_func = custom_calloc_func, .free_func = custom_free_func, .context = &mmem};


/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
RESOURCE(res_os_hello,
         "title=\"Hello world: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /*//OSCOAP
  coap_packet_t* coap_request = (coap_packet_t*)request;
  if(IS_OPTION(coap_request, COAP_OPTION_OBJECT_SECURITY)){
    coap_packet_t* coap_response = (coap_packet_t*)response;
    coap_response->context = coap_request->context;
    coap_set_header_object_security(coap_response);
    printf("OSCOAP!\n");
  }else {
    printf("NOT OSCOAP\n");
    printf("TODO SEND ERRORS!\n");
  }*/


  const char *len = NULL;
  /* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */
  char const *const message = "Hello World! ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy";
  int length = 12; /*           |<-------->| */

  /* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
  if(REST.get_query_variable(request, "len", &len)) {
    length = atoi(len);
    if(length < 0) {
      length = 0;
    }
    if(length > REST_MAX_CHUNK_SIZE) {
      length = REST_MAX_CHUNK_SIZE;
    }
    memcpy(buffer, message, length);
  } else {
    memcpy(buffer, message, length);
  } 

  const byte key[128];
  memset(key, 0, 128);
  //key = calloc(128, sizeof(byte));  //All zero?
  
  length = encrypt(buffer, message, length, key, 128, preferred_size);
  
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_etag(response, (uint8_t *)&length, 1);
  REST.set_response_payload(response, buffer, length);
}

int encrypt(uint8_t *buffer, const uint8_t *message, size_t len, const byte *key, size_t szKey, uint16_t prefsz) {
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
  objcose = COSE_Encrypt_Init(COSE_INIT_FLAGS_ZERO_FORM, &ctx,&err);

  if( objcose == NULL ) {
    PRINTF("Error in init cose: %i\n", err.err);
  }

  if( !COSE_Encrypt_SetContent(objcose, message, len, &err)){
    PRINTF("Error in set content cose: %i\n",err.err);
  }

  if( !COSE_Encrypt_encrypt(objcose, key, szKey, &err)){
    PRINTF("Error in encrypt cose: %i\n", err.err);
  }

  return COSE_Encode(objcose, buffer, prefsz, prefsz);
  
}
