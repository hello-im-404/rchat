#ifndef SERVER_CLIENTS_H
#define SERVER_CLIENTS_H

#include "../common/buffer.h"

#define MAX_CLIENTS 10000

typedef struct {
    int socket;
    char nick[64];
    char room[64];
    int is_admin;
    int needs_auth;
    char ip[32];
    ByteBuffer buf;
} Client;

extern Client clients[MAX_CLIENTS];
extern char banned_ips[100][32];
extern int banned_count;

void clients_init(void);
void clients_add(int socket, const char *ip, const char *nick);
void clients_remove(int index);
int clients_is_banned(const char *ip);
void clients_ban_ip(const char *ip);
int clients_find_by_socket(int socket);

#endif
