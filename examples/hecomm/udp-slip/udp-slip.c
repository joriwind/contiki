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


static struct udp_socket udp_listener;


PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);



void udp_socket_callback(struct udp_socket *c, void *ptr, const uip_ipaddr_t *source_addr,
                                             uint16_t source_port,
                                             const uip_ipaddr_t *dest_addr,
                                             uint16_t dest_port,
                                             const uint8_t *data,
                                             uint16_t datalen){
  PRINTF("UDP SOCKET CALLBACK\n");
  //We have a packet that needs to be send via slipnet/serial communication
  slipnet_input();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

  PROCESS_BEGIN();
  PROCESS_PAUSE();
  SENSORS_ACTIVATE(button_sensor);
  
  PRINTF("udp-slip started\n");
  
  //Start serial communication
  slipnet_init();

  //Setup udp socket listener
  if(!udp_socket_register(&udp_listener, NULL, udp_socket_callback)){
    PRINTF("Unable to register UDP Socket\n");
    return -1;
  }

  if(!udp_socket_bind(&udp_listener, UDP_LISTEN_PORT)){
    PRINTF("Unable to bind UDP socket to port: %u\n", UDP_LISTEN_PORT);
    return -1;
  }
  PRINTF("UDP SOCKET IS INITIALISED!\n");


  while(1) {
    //PROCESS_YIELD();
    PROCESS_WAIT_EVENT();
    PRINTF("Wait ended!\n");
    PRINTF("Event: %u", (uint16_t)ev);
    if(ev == tcpip_event) {
      PRINTF("TCPIP EVENT\n");
      /* tcpip_handler(); */
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
