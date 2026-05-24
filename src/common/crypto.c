#include "crypto.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PADDING_BLOCK 256
#define MAX_SEEN_NONCES 2048

static unsigned char seen_nonces[MAX_SEEN_NONCES][crypto_secretbox_NONCEBYTES];
static int seen_nonces_idx = 0;

void crypto_init(void) {
    if (sodium_init() < 0) {
        fprintf(stderr, "Failed to initialize libsodium\n");
        exit(EXIT_FAILURE);
    }
    memset(seen_nonces, 0, sizeof(seen_nonces));
}

void crypto_hash_password(const char *password, unsigned char *key_out) {
    if (strlen(password) == 0) {
        memset(key_out, 0, crypto_secretbox_KEYBYTES);
        return;
    }
    crypto_generichash(key_out, crypto_secretbox_KEYBYTES,
                       (const unsigned char *)password, strlen(password),
                       NULL, 0);
}

int crypto_encrypt_message(const unsigned char *key, const char *message, unsigned char *ciphertext_out, unsigned long long *ciphertext_len_out) {
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    int orig_len = strlen(message);
    int padded_len = orig_len;
    if (padded_len % PADDING_BLOCK != 0) {
        padded_len = padded_len + (PADDING_BLOCK - (padded_len % PADDING_BLOCK));
    }
    
    unsigned char *padded_msg = calloc(1, padded_len + 1);
    memcpy(padded_msg, message, orig_len);
    
    memcpy(ciphertext_out, nonce, sizeof(nonce));
    
    crypto_secretbox_easy(ciphertext_out + sizeof(nonce), 
                          padded_msg, padded_len, 
                          nonce, key);
                          
    *ciphertext_len_out = sizeof(nonce) + padded_len + crypto_secretbox_MACBYTES;
    free(padded_msg);
    return 0;
}

int crypto_decrypt_message(const unsigned char *key, const unsigned char *ciphertext, unsigned long long ciphertext_len, char *message_out) {
    if (ciphertext_len < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
        return -1;
    }

    const unsigned char *nonce = ciphertext;
    
    for (int i = 0; i < MAX_SEEN_NONCES; i++) {
        if (memcmp(seen_nonces[i], nonce, crypto_secretbox_NONCEBYTES) == 0) {
            return -2; 
        }
    }

    const unsigned char *c = ciphertext + crypto_secretbox_NONCEBYTES;
    unsigned long long c_len = ciphertext_len - crypto_secretbox_NONCEBYTES;

    if (crypto_secretbox_open_easy((unsigned char *)message_out, c, c_len, nonce, key) != 0) {
        return -1; 
    }
    
    message_out[c_len - crypto_secretbox_MACBYTES] = '\0';
    
    memcpy(seen_nonces[seen_nonces_idx], nonce, crypto_secretbox_NONCEBYTES);
    seen_nonces_idx = (seen_nonces_idx + 1) % MAX_SEEN_NONCES;
    
    return 0;
}
