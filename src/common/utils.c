#include "utils.h"
#include <stdio.h>
#include <string.h>

void to_hex(const unsigned char *in, int len, char *out) {
    for (int i = 0; i < len; i++) {
        sprintf(out + (i * 2), "%02x", in[i]);
    }
    out[len * 2] = '\0';
}

int from_hex(const char *in, unsigned char *out) {
    int len = strlen(in);
    if (len % 2 != 0) return -1;
    for (int i = 0; i < len / 2; i++) {
        sscanf(in + i * 2, "%02hhx", &out[i]);
    }
    return len / 2;
}
