/*
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
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"

#include "net/ip/udp-socket.h"
#include "slip-net.h"

#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG 1
#include "net/ip/uip-debug.h"
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_LISTEN_PORT	5683
#define UDP_REMOTE_PORT 5683
#define FOG_INTERNAL_ADDRESS_CONF {0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1}
#define FOG_INTERNAL_PORT 5683


static struct udp_socket udp_listener, udp_send;
static struct uip_udp_conn *udpconn;
  uip_ipaddr_t fogIpAddr;



PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);



void udp_socket_callback(struct udp_socket *c, void *ptr, const uip_ipaddr_t *source_addr,
                                             uint16_t source_port,
                                             const uip_ipaddr_t *dest_addr,
                                             uint16_t dest_port,
                                             const uint8_t *data,
                                             uint16_t datalen){
  PRINTF("UDP SOCKET CALLBACK\n");
  PRINTF("Message: %s, len: %u\n", data, datalen);
  //We have a packet that needs to be send via slipnet/serial communication

  if(!udp_socket_send(&udp_send, data, datalen)){
    PRINTF("Unable to send to FOG INTERNAL\n");
  }else{
    PRINTF("SEND data to fog\n");
  }
}

void event_handler(){
  char *appdata;

  if(uip_newdata()) {
    //Received part
    appdata = (char *)uip_appdata;
    appdata[uip_datalen()] = 0;
    PRINTF("DATA recv '%s' \n", appdata);

    //Add the headers back on the menu...
    //I assume the udp headers are still present in uip_buf 
    uip_len += UIP_IPUDPH_LEN;
    slipnet_output();
    //slipnet_send_data((uint8_t *)appdata, uip_datalen());

    //Replying part
    PRINTF("DATA sending reply\n");
    //uip_ipaddr_copy(&udpconn->ripaddr, &UIP_IP_BUF->srcipaddr);
    //Packet was send to me, now sending to fog
    //uip_ipaddr_copy(&udpconn->ripaddr, &fogIpAddr);
    //tcpip_input();
     //uip_udp_packet_send(udpconn, "Reply", sizeof("Reply"));
    //uip_create_unspecified(&udpconn->ripaddr); 
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

  PROCESS_BEGIN();
  PROCESS_PAUSE();
  SENSORS_ACTIVATE(button_sensor);
  
  PRINTF("udp-slip started\n");
  
  //NETSTACK_MAC.off(1);
  /*  //Start serial communication
  slipnet_init(); */

  //Setup udp socket listener
  /* if(!udp_socket_register(&udp_listener, NULL, udp_socket_callback)){
    PRINTF("Unable to register UDP Socket\n");
    return -1;
  }

  if(!udp_socket_bind(&udp_listener, UDP_LISTEN_PORT)){
    PRINTF("Unable to bind UDP socket to port: %u\n", UDP_LISTEN_PORT);
    return -1;
  } */

  udpconn = udp_new(NULL, UIP_HTONS(0), NULL); 
 
  udp_bind(udpconn, UIP_HTONS(UDP_LISTEN_PORT));
  PRINTF("UDP LISTEN IS INITIALISED!\n"); 


  uip_ip6addr(&fogIpAddr,0xff, 0,0,0,0,0,0,6);

  


  if(!udp_socket_connect(&udp_send, &fogIpAddr, FOG_INTERNAL_PORT)){
    PRINTF("Unable to connect FOG INTERNAL udp socket\n");
    return -1;
  }




    //slipnet_request_prefix();

  while(1) {
    //PROCESS_YIELD();
    PROCESS_WAIT_EVENT();
    PRINTF("Wait ended!\n");
    PRINTF("Event: %u", (uint16_t)ev);
    if(ev == tcpip_event) {
      PRINTF("TCPIP EVENT\n");
      /* tcpip_handler(); */
      event_handler();
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
