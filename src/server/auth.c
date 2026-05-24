#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sodium.h>

typedef struct {
    char nick[64];
    char hash[128];
    time_t last_login;
} UserRecord;

static void double_hash(const char *client_hash, char *out_hash) {
    unsigned char bin_hash[crypto_generichash_BYTES];
    crypto_generichash(bin_hash, sizeof(bin_hash), (const unsigned char *)client_hash, strlen(client_hash), NULL, 0);
    for (size_t i = 0; i < sizeof(bin_hash); i++) {
        sprintf(out_hash + (i * 2), "%02x", bin_hash[i]);
    }
    out_hash[sizeof(bin_hash) * 2] = '\0';
}

void auth_init(void) {
    if (sodium_init() < 0) exit(EXIT_FAILURE);
    auth_cleanup_expired();
}

int auth_is_registered(const char *nick) {
    FILE *f = fopen(AUTH_DB_FILE, "r");
    if (!f) return 0;
    
    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char f_nick[64];
        if (sscanf(line, "%63s", f_nick) == 1) {
            if (strcmp(f_nick, nick) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(f);
    return found;
}

int auth_login(const char *nick, const char *client_hash) {
    FILE *f = fopen(AUTH_DB_FILE, "r");
    if (!f) return 0;
    
    char line[512];
    char expected_hash[128] = {0};
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char f_nick[64];
        long long ts;
        if (sscanf(line, "%63s %127s %lld", f_nick, expected_hash, &ts) == 3) {
            if (strcmp(f_nick, nick) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(f);
    
    if (!found) return 0;
    
    char d_hash[128];
    double_hash(client_hash, d_hash);
    
    if (strcmp(expected_hash, d_hash) == 0) {
        auth_update_timestamp(nick);
        return 1;
    }
    return 0;
}

int auth_register(const char *nick, const char *client_hash) {
    if (auth_is_registered(nick)) return 0;
    
    FILE *f = fopen(AUTH_DB_FILE, "a");
    if (!f) return 0;
    
    char d_hash[128];
    double_hash(client_hash, d_hash);
    
    fprintf(f, "%s %s %lld\n", nick, d_hash, (long long)time(NULL));
    fclose(f);
    return 1;
}

void auth_update_timestamp(const char *nick) {
    FILE *f = fopen(AUTH_DB_FILE, "r");
    if (!f) return;
    
    FILE *temp = fopen(AUTH_DB_FILE ".tmp", "w");
    if (!temp) {
        fclose(f);
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char f_nick[64], f_hash[128];
        long long ts;
        if (sscanf(line, "%63s %127s %lld", f_nick, f_hash, &ts) == 3) {
            if (strcmp(f_nick, nick) == 0) {
                fprintf(temp, "%s %s %lld\n", f_nick, f_hash, (long long)time(NULL));
            } else {
                fprintf(temp, "%s %s %lld\n", f_nick, f_hash, ts);
            }
        }
    }
    
    fclose(f);
    fclose(temp);
    rename(AUTH_DB_FILE ".tmp", AUTH_DB_FILE);
}

void auth_cleanup_expired(void) {
    FILE *f = fopen(AUTH_DB_FILE, "r");
    if (!f) return;
    
    FILE *temp = fopen(AUTH_DB_FILE ".tmp", "w");
    if (!temp) {
        fclose(f);
        return;
    }
    
    long long now = (long long)time(NULL);
    long long expire_secs = EXPIRE_DAYS * 24 * 3600;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char f_nick[64], f_hash[128];
        long long ts;
        if (sscanf(line, "%63s %127s %lld", f_nick, f_hash, &ts) == 3) {
            if (now - ts <= expire_secs) {
                fprintf(temp, "%s %s %lld\n", f_nick, f_hash, ts);
            }
        }
    }
    
    fclose(f);
    fclose(temp);
    rename(AUTH_DB_FILE ".tmp", AUTH_DB_FILE);
}
