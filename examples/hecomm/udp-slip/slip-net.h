#ifndef __SLIP_NET_H__
#define __SLIP_NET_H__

/*
 * This will send packet into slip 
   copy into uip-but and send!!! 
   Format: !R<data> ? 
 */
void slipnet_input(void);


void slipnet_init();

void slipnet_output(void);

void slipnet_request_prefix(void);

void slipnet_send_data(uint8_t *data, uint16_t len);

const struct uip_fallback_interface rpl_interface;

#endif