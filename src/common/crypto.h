#ifndef COMMON_CRYPTO_H
#define COMMON_CRYPTO_H
#include <sodium.h>

#define MAX_CIPHERTEXT_LEN 8192

void crypto_init(void);
void crypto_hash_password(const char *password, unsigned char *key_out);
int crypto_encrypt_message(const unsigned char *key, const char *message, unsigned char *ciphertext_out, unsigned long long *ciphertext_len_out);
int crypto_decrypt_message(const unsigned char *key, const unsigned char *ciphertext, unsigned long long ciphertext_len, char *message_out);

#endif
