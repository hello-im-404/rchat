#include "clients.h"
#include <string.h>
#include <unistd.h>

Client clients[MAX_CLIENTS];
char banned_ips[100][32];
int banned_count = 0;

void clients_init(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        buffer_init(&clients[i].buf, 1024);
    }
}

void clients_add(int socket, const char *ip, const char *nick) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = socket;
            strncpy(clients[i].nick, nick, 63);
            strncpy(clients[i].ip, ip, 31);
            clients[i].room[0] = '\0';
            clients[i].is_admin = 0;
            clients[i].needs_auth = 0;
            clients[i].buf.size = 0; 
            break;
        }
    }
}

void clients_remove(int index) {
    if (clients[index].socket > 0) {
        close(clients[index].socket);
        clients[index].socket = 0;
    }
}

int clients_is_banned(const char *ip) {
    for (int i = 0; i < banned_count; i++) {
        if (strcmp(banned_ips[i], ip) == 0) return 1;
    }
    return 0;
}

void clients_ban_ip(const char *ip) {
    if (banned_count < 100 && !clients_is_banned(ip)) {
        strncpy(banned_ips[banned_count++], ip, 31);
    }
}

int clients_find_by_socket(int socket) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == socket) return i;
    }
    return -1;
}
