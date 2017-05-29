#include "contiki.h"

#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "ospad.h"
#include "rest-engine.h"

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

static void res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple actuator example. Toggles the red led */
RESOURCE(res_compartner,
         "title=\"Communication partner ?address=value\";rt=\"Control\"",
         NULL,
         res_post_handler,
         NULL,
         NULL);

static void
res_post_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    size_t len = 0;
    uip_ipaddr_t addr;
    const char *bytes = NULL;
    uint16_t integers[8];
    int success = 0;
    int i;
    PRINTF("Communication partner post\n");
    if((len = REST.get_request_payload(request, &bytes))) {
        if(len != 16){
            PRINTF("Wrong lenght of address: %i\n", len);
        }
        //PRINTF("Communication partner: %.*s\n", len, bytes);
        //Conversion to u16
        PRINTF("Remap \n");
        /*uint8_t buf1;
        uint8_t buf2;
        for (i = 0; i<8; i++){
            sscanf(&bytes[2*i], "%02x", &buf1);
            sscanf(&bytes[2*i + 1], "%02x", &buf2);
            integers[i] = buf1 << 8;
            integers[i] |= buf2;
            PRINTF("Integer %i: %i ", i, integers[i]);
        }*/
        uiplib_ip6addrconv(bytes, &addr);
        PRINTF("\n");
        PRINTF("Creating address \n");
        //Create address
        //uip_ip6addr(&addr, integers[0], integers[1], integers[2], integers[3], integers[4], integers[5], integers[6], integers[7]);
        //Set communication partner in ospad
        PRINTF("Setting address \n");
        setCommunicationPartner(&addr);
        
        PRINTF("Printing the address: \n");
        PRINT6ADDR(&addr);
        success = 1;
    }
    if(!success) {
        REST.set_response_status(response, REST.status.BAD_REQUEST);
    }else{
        REST.set_response_status(response, REST.status.OK);
    }
}