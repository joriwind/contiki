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
 *      Erbium (Er) CoAP client example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#if COAP_ENABLED
  #include "rest-engine.h"
  #include "er-coap-engine.h"
#endif
#if COSE_ENABLED
  #include "obj-sec.h"
  #ifndef USE_MEMB
    #include "mmem.h"
  #endif
#endif
#include "dev/button-sensor.h"

#include "cc2420.h"
#include "ccm-star.h"

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

uint16_t i;
#define PRINTBUF(buffer, begin, end)  printf("Buffer:{"); for (i = begin; i < end; i++){ \
	printf(" %x", buffer[i]);  \
  } \
  printf("}\n");

/* FIXME: This server address is hard-coded for Cooja and link-local for unconnected border router. */
//#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7402, 0x0002, 0x0202)      /* cooja2 */
//#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7402, 0x0002, 0x0202)      /* cooja2 */
//#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfd6d, 0xf18e, 0x19a8, 0, 0, 0, 0, 0x0c33)
//#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfd6d, 0xf18e, 0x19a8, 0, 0x6119, 0x12dc, 0x4ff4, 0x6b9a)

#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x1)
#define FOG_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0xc30c, 0, 0, 0x5)
/* #define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1) */

#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#if COAP_ENABLED
  #if HECOMM_ENABLED
    extern resource_t res_hello, res_os_hello, res_os_key, res_compartner;
  #else
    extern resource_t res_hello, res_compartner;
  #endif
#endif


/*uint8_t sender_id[] =  { 0x73, 0x65, 0x72, 0x76, 0x65, 0x72 };
uint8_t receiver_id[] = { 0x63, 0x6C, 0x69, 0x65, 0x6E, 0x74 };*/
uint8_t sender_id[] =  { 0, 0, 0, 0, 0, 0 };
uint8_t receiver_id[] = { 0, 0, 0, 0, 0, 0 };

/* uint8_t master_secret[35] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 
            0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 
            0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23};  */


PROCESS(node_cose, "Erbium Example Client: node Cose");
AUTOSTART_PROCESSES(&node_cose);

uip_ipaddr_t server_ipaddr;
uip_ipaddr_t fog_ipaddr;

#if COAP_ENABLED
  /* Example URIs that can be queried. */
  #define NUMBER_OF_URLS 3
  /* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
  char *service_urls[NUMBER_OF_URLS] =
  { ".well-known/core", "/hello", "/req" };

  /* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
  void
  client_chunk_handler(void *response)
  {
    const uint8_t *chunk;

    int len = coap_get_payload(response, &chunk);

    printf("|%.*s", len, (char *)chunk);
  }
  void
  client_secured_handler(void *response)
  {
    const uint8_t *chunk;

    int len = coap_get_payload(response, &chunk);
    
    printf("Secured payload|%.*s", len, (char *)chunk);
  }
#endif

PROCESS_THREAD(node_cose, ev, data)
{
  PROCESS_BEGIN();

#if COAP_ENABLED
  static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

  SERVER_NODE(&server_ipaddr);
  FOG_NODE(&fog_ipaddr);

  #if COSE_ENABLED
    #ifndef USE_MEMB
     mmem_init();
    #endif

    objsec_init();
  #endif
#endif

#if COAP_ENABLED
  /* Initialize the REST engine. */
  rest_init_engine();
  rest_activate_resource(&res_hello, "test/hello");
  #if HECOMM_ENABLED
  rest_activate_resource(&res_os_hello, "test/os-hello");
  rest_activate_resource(&res_os_key, "hecomm/osskey");
  #endif
  rest_activate_resource(&res_compartner, "hecomm/commpartner");

  /* receives all CoAP messages */
  coap_init_engine();

#if PLATFORM_HAS_BUTTON
  SENSORS_ACTIVATE(button_sensor);
  printf("Press a button to request %s\n", service_urls[1]);
#endif

  while(1) {
    PROCESS_WAIT_EVENT();
#if PLATFORM_HAS_BUTTON
     if(ev == sensors_event && data == &button_sensor) {
      /* printf("\n");
      //Testing aes driver and cmm star
      printf("START OF TEST\n");
      uint8_t key[16] = {0};
      uint8_t start[16] = { 0x0, 0x1, 0x2, 0x3, 
                            0x4, 0x5, 0x6, 0x7, 
                            0x8, 0x9, 0xA, 0xB,
                            0xC, 0xD, 0xE, 0xF};
      uint8_t iv[16] = {0};
      uint8_t buffer[16];
      uint8_t tag[16];
      memcpy(buffer, start, 16);
      set_key1(key);
      en_key(1);

      printf("Buffer before aes encrypt: ");
      PRINTBUF(buffer, 0, 16);
      printf("Using key: %x\n", read_SECCTRL0());
      cc2420_aes_128_driver.encrypt(buffer);
      printf("Buffer after aes encrypt: ");
      PRINTBUF(buffer, 0, 16);
      memcpy(buffer, start, 16);

      printf("Buffer before mic encrypt: ");
      PRINTBUF(buffer, 0, 16);
      printf("Using key: %x\n", read_SECCTRL0());
	    CCM_STAR.mic(buffer, 16, iv, 0,  0, tag, 16);
      memcpy(buffer, tag, 16);
      memcpy(tag, iv, 16);

      printf("Buffer after mic encrypt: ");
      PRINTBUF(buffer, 0, 16);

      en_key(0);

      memcpy(buffer, start, 16);

      printf("Buffer before aes encrypt: ");
      PRINTBUF(buffer, 0, 16);
      printf("Using key: %x\n", read_SECCTRL0());
      cc2420_aes_128_driver.encrypt(buffer);
      printf("Buffer after aes encrypt: ");
      PRINTBUF(buffer, 0, 16);
      memcpy(buffer, start, 16);

      printf("Buffer before encrypt: ");
      PRINTBUF(buffer, 0, 16);

      printf("Using key: %x\n", read_SECCTRL0());
	    CCM_STAR.mic(buffer, 16, iv, 0,  0, tag, 16);
      memcpy(buffer, tag, 16);
      memcpy(tag, iv, 16);

      printf("Buffer after encrypt: ");
      PRINTBUF(buffer, 0, 16); */




      //Debugging  COSE encrypt and decrypt
      /* printf("\n");
      printf("Testing cose encrypt decrypt\n");
      size_t preferred_size = 128;
      uint8_t buffer[preferred_size];

      size_t plaintextLength = 128;
      uint8_t plaintext[plaintextLength];

      char const *const message = "Hello World!";
      size_t messageLength = 12;
      size_t cipherLength;
      printf("Sending: %s, length: %u\n", message, messageLength);
      printf("Encrypting... provided buffer of size: %u\n", preferred_size);
      cipherLength = encrypt(buffer,preferred_size, (const uint8_t *) message, messageLength);
      if(cipherLength < 0){
        printf("Encrypting failed\n");
        break;
      }
      printf("Decrypting...\n");
      //decrypt function provides output array in plaintext
      plaintextLength = decrypt(buffer, cipherLength, plaintext, plaintextLength);
      if(plaintextLength < 0){
        printf("Decrypting failed\n");
        break;
      }
      printf("Decrypted message: %s, length: %u\n", plaintext, plaintextLength); */





      if(!objsec_key_set()){
        //Sending a key request to 6lowpan network manager
        uint8_t inftype[1] = {1};
        coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
        coap_set_header_uri_path(request, service_urls[2]);
        coap_set_payload(request, &inftype, sizeof(inftype));

        printf("--Requesting %s--\n", service_urls[2]);

        PRINT6ADDR(&server_ipaddr);
        PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

        COAP_BLOCKING_REQUEST(&server_ipaddr, REMOTE_PORT, request,
                              client_chunk_handler);

        printf("\n--Done--\n");
      }else{
        //Key is set thus sending request to fog, to link
        
        coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
        coap_set_header_uri_path(request, service_urls[1]);

        printf("--Requesting %s--\n", service_urls[1]);

        PRINT6ADDR(&server_ipaddr);
        PRINTF(" : %u\n", UIP_HTONS(REMOTE_PORT));

        COAP_BLOCKING_REQUEST(&fog_ipaddr, REMOTE_PORT, request,
                              client_secured_handler);

        printf("\n--Done--\n");

      }

    }
#endif
  }
#endif

  PROCESS_END();
}
