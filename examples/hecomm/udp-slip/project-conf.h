
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_


 #define NETSTACK_CONF_WITH_IPV6 1


/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    256

/* Increase rpl-border-router IP-buffer when using more than 64. */
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE            64//48

#undef NETSTACK_CONF_LLSEC
#define NETSTACK_CONF_LLSEC noncoresec_driver

#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED 1
#ifndef LLSEC802154_CONF_SECURITY_LEVEL
#define LLSEC802154_CONF_SECURITY_LEVEL 7
#endif


#define NONCORESEC_CONF_KEY { 0x00 , 0x01 , 0x02 , 0x03 , \
                              0x04 , 0x05 , 0x06 , 0x07 , \
                              0x08 , 0x09 , 0x0A , 0x0B , \
                              0x0C , 0x0D , 0x0E , 0x0F }


#endif PROJECT_CONF_H_