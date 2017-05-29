//#define DEBUG 0

#include "ospad.h"
#include <stdio.h>
#include "sys/cc.h"
#include "contiki.h"
#include <string.h>

#ifdef OSPAD_CONF_KEY
#define OSPAD_KEY OSPAD_CONF_KEY
#else
#define OSPAD_KEY     { 0x00 , 0x00 , 0x00 , 0x00 , \
                        0x00 , 0x00 , 0x00 , 0x00 , \
                        0x00 , 0x00 , 0x00 , 0x00 , \
                        0x00 , 0x00 , 0x00 , 0x00 }
#endif /* OSPAD_CONF_KEY */

#define OSPAD_KEY_SIZE 32

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

//Object security session key
uint8_t osskey[OSPAD_KEY_SIZE] = OSPAD_KEY;
uint16_t keySz = OSPAD_KEY_SIZE;
uint8_t usages;

//Define the communication partner
uip_ipaddr_t partner;

uint8_t ospad(char* data, uint16_t sz)
{
    int i;
    PRINTF("ospad: padding data!\n");
    //Control checks
    if(keySz < sz){
        PRINTF("ospad: data longer than key!!\n");
        return OSPAD_LENGTH_ERROR;
    }

    if(usages > 0){
        PRINTF("ospad: ALREADY USED KEY!!\n");
        return OSPAD_GENERAL_ERROR;
    }

    for(i = 0; i < sz; i++){
        data[i] = data[i] ^ osskey[i]; //XOR message and key
    }
    usages++;

    return OSPAD_SUCCES;
}

/*
 * Set the key of ospad
 */
uint8_t setOSSKey(char* key, uint16_t sz){
    if(sz > OSPAD_KEY_SIZE){
        PRINTF("Key to large!\n");
        return OSPAD_GENERAL_ERROR;
    }
    PRINTF("ospad: Setting new key: %.*s, length: %i\n", sz, key, sz);
    keySz = sz;
    memcpy(osskey, key, sz);
    //Reset usages
    usages = 0;
    return 0;
}

//Returns the amount of usages of this key
uint8_t getUsages(){
    return usages;
}

//@return return the identification of the communication partner
uip_ipaddr_t* getCommunicationPartner(){
    return &partner;
}

//Set the communication partner
void setCommunicationPartner(uip_ipaddr_t *addr){
    PRINTF("ospad: setting new communication partner\n");
    memcpy(&partner, addr, sizeof(uip_ipaddr_t));
}
