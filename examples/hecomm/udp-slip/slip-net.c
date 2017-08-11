#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "dev/slip.h"
#include "dev/uart1.h"
#include <string.h>
#include "net/rpl/rpl.h"
#include "net/netstack.h"

#define DEBUG 1
#include "net/ip/uip-debug.h"
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define UIP_IP_BUF        ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static uip_ipaddr_t last_sender;

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335


void set_prefix_64(uip_ipaddr_t *);

static void
slip_input_callback(void)
{
 // PRINTF("SIN: %u\n", uip_len);
  if(uip_buf[0] == '!') {
    PRINTF("Got configuration message of type %c\n", uip_buf[1]);
    uip_len = 0;
    if(uip_buf[1] == 'P') {
      uip_ipaddr_t prefix;
      /* Here we set a prefix !!! */
      memset(&prefix, 0, 16);
      memcpy(&prefix, &uip_buf[2], 8);
      PRINTF("Setting prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      set_prefix_64(&prefix);
    }
  } else if (uip_buf[0] == '?') {
    PRINTF("Got request message of type %c\n", uip_buf[1]);
    if(uip_buf[1] == 'M') {
      char* hexchar = "0123456789abcdef";
      int j;
      /* this is just a test so far... just to see if it works */
      uip_buf[0] = '!';
      for(j = 0; j < 8; j++) {
        uip_buf[2 + j * 2] = hexchar[uip_lladdr.addr[j] >> 4];
        uip_buf[3 + j * 2] = hexchar[uip_lladdr.addr[j] & 15];
      }
      uip_len = 18;
      slip_send();
      
    }
    uip_len = 0;
  }
  /* Save the last sender received over SLIP to avoid bouncing the
     packet back if no route is found */
  uip_ipaddr_copy(&last_sender, &UIP_IP_BUF->srcipaddr);
}

void slipnet_init()
{
  slip_arch_init(BAUD2UBR(115200));
  process_start(&slip_process, NULL);
  slip_set_input_callback(slip_input_callback);


  /* Here we set a prefix !!! */
  uip_ip6addr(&prefix,0xbbbb, 0,0,0,0,0,0,1);
  PRINTF("Setting prefix ");
  PRINT6ADDR(&prefix);
  PRINTF("\n");
  set_prefix_64(&prefix);
}

void slipnet_output(void)
{
  if(uip_ipaddr_cmp(&last_sender, &UIP_IP_BUF->srcipaddr)) {
    /* Do not bounce packets back over SLIP if the packet was received
       over SLIP */
    PRINTF("slip-bridge: Destination off-link but no route src=");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF(" dst=");
    PRINT6ADDR(&UIP_IP_BUF->destipaddr);
    PRINTF("\n");
  } else {
    PRINTF("SUT: %u\n", uip_len);
#ifdef CONF_DIRTY_REDIRECT_HACK
    uint8_t fixedHdr[4] = {0x60, 0x00, 0x00, 0x00};//, 0x00, 0x1e, 0x11
    uint8_t *ptr;
    uint8_t i;
    ptr = &uip_buf[UIP_LLH_LEN];

    for(i = 0; i<sizeof(fixedHdr) ; i++)
      ptr[i] = fixedHdr[i];
#endif
    slip_send();
  }
}

#if !SLIP_BRIDGE_CONF_NO_PUTCHAR
#undef putchar
int
putchar(int c)
{
#define SLIP_END     0300
  static char debug_frame = 0;

  if(!debug_frame) {            /* Start of debug output */
    slip_arch_writeb(SLIP_END);
    slip_arch_writeb('\r');     /* Type debug line == '\r' */
    debug_frame = 1;
  }

  /* Need to also print '\n' because for example COOJA will not show
     any output before line end */
    if(c == SLIP_END) {
      slip_arch_writeb(SLIP_ESC);
      c = SLIP_ESC_END;
    } else if(c == SLIP_ESC) {
      slip_arch_writeb(SLIP_ESC);
      c = SLIP_ESC_ESC;
    }
    slip_arch_writeb(c);

  /*
   * Line buffered output, a newline marks the end of debug output and
   * implicitly flushes debug output.
   */
  if(c == '\n') {
    slip_arch_writeb(SLIP_END);
    debug_frame = 0;
  }
  return c;
}
#endif

void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  /* dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
  if(dag != NULL) {
    rpl_set_prefix(dag, &prefix, 64);
    PRINTF("created a new RPL dag\n");
  } */
}

void slipnet_request_prefix(void)
{
  /* mess up uip_buf with a dirty request... */
  uip_buf[0] = '?';
  uip_buf[1] = 'P';
  uip_len = 2;
  slip_send();
  uip_len = 0;
}

const struct uip_fallback_interface rpl_interface = {
  slipnet_init, slipnet_output
};

/*
 *  Used to send data directly using slip protocol.
 *  Sending: SLIP_END -> DATA -> SLIP_END
 */
void slipnet_send_data(uint8_t *data, uint16_t len){
  
  PRINTF("Sending data over SLIP\n");
    PRINTF("SUT: %u\n", uip_len);
  uint8_t i;
  uint8_t c;
  //Start byte
  slip_arch_writeb(SLIP_END);

  //Send all the data
  for(i = 0; i < len; i++){
    //slip_arch_writeb((char)data[i]);
    c = data[i];
    if(c == SLIP_END) {
      slip_arch_writeb(SLIP_ESC);
      c = SLIP_ESC_END;
    } else if(c == SLIP_ESC) {
      slip_arch_writeb(SLIP_ESC);
      c = SLIP_ESC_ESC;
    }
    slip_arch_writeb(c);
  }

  

  //End byte
  slip_arch_writeb(SLIP_END);
}
