#ifndef __OBJ_SEC_H__
#define __OBJ_SEC_H__

#include <stddef.h>
#include <contiki.h>

uint8_t objsec_key_set();

void objsec_init();
void objsec_set_key(uint8_t * k);

/**
 * Used to abstract the cose encryption/ key management
 * @param buffer    Buffer to push the ciphertext into
 * @param message   Input message, to be encrypted
 * @param len       Length of message
 * @param prefsz    Length of buffer
 */
size_t encrypt(uint8_t *buffer, uint16_t prefsz, const uint8_t *message, size_t len);
/**
 * Used to abstract the cose encryption/ key management
 * @param buffer    Buffer to push the plaintext into
 * @param message   Input message, to be decrypted
 * @param len       Length of message
 * @return  Length of plaintext, -1 if error occured
 */
size_t decrypt(const uint8_t *message, size_t len, uint8_t **buffer);

#endif