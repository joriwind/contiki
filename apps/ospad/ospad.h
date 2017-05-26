/*
 *  Object security padding
 *  
 */
#ifndef OSPAD_H_
#define OSPAD_H_


#define ENABLE_OTP
//#define OSPAD_CONF_KEY
#include <stddef.h> 
#include <stdio.h>
#include "sys/cc.h"
#include "contiki.h"
#include "contiki-net.h"

enum _ReturnStates
{
    OSPAD_SUCCES = 0,
    OSPAD_GENERAL_ERROR = -1,
    OSPAD_LENGTH_ERROR = -2,
};

/*
 * @input   Data to be secured with one time pad key!
 */
uint8_t ospad(char* data, uint16_t sz);

/*
 * @input   New key to be used for padding data
 */
uint8_t setOSSKey(char* key, uint16_t sz);

/*
 * @return  The amount of usages of the current key
 */
uint8_t getUsages();

/*
 *
 */
uip_ipaddr_t* getCommunicationPartner();
void setCommunicationPartner(uip_ipaddr_t *addr);

#endif //OSPAD_H