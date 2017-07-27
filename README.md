# The contiki fork for the hecomm 6lowpan network

## 6LoWPAN network
**Devices:**
- [x] network manager (ipv6/rpl-border-router)
- [ ] IoT node (hecomm/node-cose)
- [ ] pkt slip radio (...)

**Security:**
* Link-layer security:    AES-CCM 128\
Standard layer of security in a 6LoWPAN network. The configuration of the mode of security can be found in frame802154.h file. The configuration header LLSEC802154_CONF_SECURITY_LEVEL can be set 0 <-> 7, 0 being no security and 7 encryption and authentication with MIC of 128. Current setting 7: FRAME802154_SECURITY_LEVEL_ENC_MIC_128.
Source: [Presentation FOSDEM 2017](https://fosdem.org/2017/schedule/event/lowpan_embedded/attachments/slides/1729/export/events/attachments/lowpan_embedded/slides/1729/FOSDEM_2017_linux_wpan.pdf)

* Network-layer security: None \
The idea was to utilise DTLS for end-to-end security in network. However, the Zolertia Z1's storage and memory size is too small.

* Object security for hecomm: COSE [RFC8152](https://tools.ietf.org/html/rfc8152)\
The security in the end-to-end communication between heterogeneous IoT networks (Hecomm system) is based on CBOR object Signing and Encryption (COSE). While the application protocol defined for the Hecomm system is CoAP, COSE is used to secure the payload of the CoAP message.

Paper: ??


The Contiki Operating System
============================

[![Build Status](https://travis-ci.org/contiki-os/contiki.svg?branch=release-3-0)](https://travis-ci.org/contiki-os/contiki/branches)

Contiki is an open source operating system that runs on tiny low-power
microcontrollers and makes it possible to develop applications that
make efficient use of the hardware while providing standardized
low-power wireless communication for a range of hardware platforms.

Contiki is used in numerous commercial and non-commercial systems,
such as city sound monitoring, street lights, networked electrical
power meters, industrial monitoring, radiation monitoring,
construction site monitoring, alarm systems, remote house monitoring,
and so on.

For more information, see the Contiki website:

[http://contiki-os.org](http://contiki-os.org)

# LLSEC security in Contiki
You need the following Contiki build options
configured in your project-conf.h to make use of
LLSEC with network wide key:
```c
#define NETSTACK_CONF_LLSEC noncoresec_driver
#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED 1
#define LLSEC802154_CONF_SECURITY_LEVEL 7
#define NONCORESEC_CONF_KEY {   0x00, 0x01, 0x02, 0x03, \
                                0x04, 0x05, 0x06, 0x07, \
                                0x08, 0x09, 0x0A, 0x0B, \
                                0x0C, 0x0D, 0x0E, 0x0F }
```

