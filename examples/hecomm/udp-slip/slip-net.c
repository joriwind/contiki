#include "contiki.h"
#include "net/netstack.h"
#include "net/ip/uip.h"
#include "net/packetbuf.h"
#include "dev/slip.h"
#include "cmd.h"
#include "packetutils.h"

#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


/*---------------------------------------------------------------------------*/
void
slip_send_packet(const uint8_t *ptr, int len)
{
  uint16_t i;
  uint8_t c;

  slip_arch_writeb(SLIP_END);
  for(i = 0; i < len; ++i) {
    c = *ptr++;
    if(c == SLIP_END) {
      slip_arch_writeb(SLIP_ESC);
      c = SLIP_ESC_END;
    } else if(c == SLIP_ESC) {
      slip_arch_writeb(SLIP_ESC);
      c = SLIP_ESC_ESC;
    }
    slip_arch_writeb(c);
  }
  slip_arch_writeb(SLIP_END);
}
/*---------------------------------------------------------------------------*/
void
slipnet_input(void)
{
  int i;
  /* radio should be configured for filtering so this should be simple */
  /* this should be sent over SLIP! */
  /* so just copy into uip-but and send!!! */
  /* Format: !R<data> ? */
  uip_len = packetbuf_datalen();
  i = packetbuf_copyto(uip_buf);

  if(DEBUG) {
    printf("Slipnet got input of len: %d, copied: %d\n",
	   packetbuf_datalen(), i);

    for(i = 0; i < uip_len; i++) {
      printf("%02x", (unsigned char) uip_buf[i]);
      if((i & 15) == 15) printf("\n");
      else if((i & 7) == 7) printf(" ");
    }
    printf("\n");
  }

  /* printf("SUT: %u\n", uip_len); */
  slip_send_packet(uip_buf, uip_len);
}


static int slip_radio_cmd_handler(const uint8_t *data, int len);
static void slip_input_callback(void);

CMD_HANDLERS(slip_radio_cmd_handler);
 /* max 16 packets at the same time??? */
uint8_t packet_ids[16];
int packet_pos = 0;

static void
packet_sent(void *ptr, int status, int transmissions)
{
  uint8_t buf[20];
  uint8_t sid;
  int pos;
  sid = *((uint8_t *)ptr);
  PRINTF("Slip-radio: packet sent! sid: %d, status: %d, tx: %d\n",
  	 sid, status, transmissions);
  /* packet callback from lower layers */
  /*  neighbor_info_packet_sent(status, transmissions); */
  pos = 0;
  buf[pos++] = '!';
  buf[pos++] = 'R';
  buf[pos++] = sid;
  buf[pos++] = status; /* one byte ? */
  buf[pos++] = transmissions;
  cmd_send(buf, pos);
}

static int
slip_radio_cmd_handler(const uint8_t *data, int len)
{
  int i;
  if(data[0] == '!') {
    /* should send out stuff to the radio - ignore it as IP */
    /* --- s e n d --- */
    if(data[1] == 'S') {
      int pos;
      packet_ids[packet_pos] = data[2];

      packetbuf_clear();
      pos = packetutils_deserialize_atts(&data[3], len - 3);
      if(pos < 0) {
        PRINTF("slip-radio: illegal packet attributes\n");
        return 1;
      }
      pos += 3;
      len -= pos;
      if(len > PACKETBUF_SIZE) {
        len = PACKETBUF_SIZE;
      }
      memcpy(packetbuf_dataptr(), &data[pos], len);
      packetbuf_set_datalen(len);

      PRINTF("slip-radio: sending %u (%d bytes)\n",
             data[2], packetbuf_datalen());

      /* parse frame before sending to get addresses, etc. */
      //framer_802154.parse();
      NETSTACK_FRAMER.parse();
      NETSTACK_LLSEC.send(packet_sent, &packet_ids[packet_pos]);

      packet_pos++;
      if(packet_pos >= sizeof(packet_ids)) {
	packet_pos = 0;
      }

      return 1;
    }
  } else if(uip_buf[0] == '?') {
    PRINTF("Got request message of type %c\n", uip_buf[1]);
    if(data[1] == 'M') {
      /* this is just a test so far... just to see if it works */
      uip_buf[0] = '!';
      uip_buf[1] = 'M';
      for(i = 0; i < 8; i++) {
        uip_buf[2 + i] = uip_lladdr.addr[i];
      }
      uip_len = 10;
      cmd_send(uip_buf, uip_len);
      return 1;
    }
  }
  return 0;
}

void slipnet_init(){
  #ifndef BAUD2UBR
#define BAUD2UBR(baud) baud
#endif
  slip_arch_init(BAUD2UBR(115200));
  process_start(&slip_process, NULL);
  slip_set_input_callback(slip_input_callback);
  packet_pos = 0;
}

static void
slip_input_callback(void)
{
  PRINTF("SR-SIN: %u '%c%c'\n", uip_len, uip_buf[0], uip_buf[1]);
  cmd_input(uip_buf, uip_len);
  uip_len = 0;
}

void
slip_radio_cmd_output(const uint8_t *data, int data_len)
{
  slip_send_packet(data, data_len);
}

