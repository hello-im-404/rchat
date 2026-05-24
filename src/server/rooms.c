#include "rooms.h"
#include "clients.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

static void send_to_client(int sock, const char *msg) {
    char buf[8192];
    snprintf(buf, sizeof(buf), "%s\n", msg);
    send(sock, buf, strlen(buf), 0);
}

void rooms_handle_join(int client_index, const char *room) {
    strncpy(clients[client_index].room, room, 63);
    
    int count = 0;
    for (int j = 0; j < MAX_CLIENTS; j++) {
        if (clients[j].socket > 0 && strcmp(clients[j].room, room) == 0) {
            count++;
        }
    }
    
    if (count == 1) {
        clients[client_index].is_admin = 1;
        send_to_client(clients[client_index].socket, "[SERVER] You created the room and are now the admin.");
    } else {
        clients[client_index].is_admin = 0;
        send_to_client(clients[client_index].socket, "[SERVER] Joined room successfully.");
    }
}

void rooms_handle_ban(int client_index, const char *target_nick) {
    if (!clients[client_index].is_admin) {
        send_to_client(clients[client_index].socket, "[SERVER] You are not an admin.");
        return;
    }
    
    int found = 0;
    for (int j = 0; j < MAX_CLIENTS; j++) {
        if (clients[j].socket > 0 && j != client_index && 
            strcmp(clients[j].room, clients[client_index].room) == 0 && 
            strcmp(clients[j].nick, target_nick) == 0) {
            
            clients_ban_ip(clients[j].ip);
            send_to_client(clients[j].socket, "[SERVER] You have been banned by the admin.");
            clients_remove(j);
            send_to_client(clients[client_index].socket, "[SERVER] User banned successfully.");
            found = 1;
        }
    }
    if (!found) {
        send_to_client(clients[client_index].socket, "[SERVER] User not found in this room.");
    }
}

void rooms_handle_msg(int client_index, const char *msg) {
    if (strlen(clients[client_index].room) == 0) return;
    
    char broadcast[8192];
    snprintf(broadcast, sizeof(broadcast), "/msg %s %s", clients[client_index].nick, msg);
    
    for (int j = 0; j < MAX_CLIENTS; j++) {
        if (clients[j].socket > 0 && j != client_index && strcmp(clients[j].room, clients[client_index].room) == 0) {
            send_to_client(clients[j].socket, broadcast);
        }
    }
}
