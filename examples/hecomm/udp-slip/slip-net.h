#ifndef __SLIP_NET_H__
#define __SLIP_NET_H__

/*
 * This will send packet into slip 
   copy into uip-but and send!!! 
   Format: !R<data> ? 
 */
void slipnet_input(void);


void slipnet_init();

#endif