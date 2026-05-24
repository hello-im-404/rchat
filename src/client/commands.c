#include "commands.h"
#include "state.h"
#include "network.h"
#include "ui.h"
#include "../common/crypto.h"
#include "../common/utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void save_server_to_history(const char *addr, int port) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.rchat_servers", getenv("HOME"));
    
    FILE *f = fopen(path, "a+");
    if (!f) return;
    
    char line[512];
    int found = 0;
    fseek(f, 0, SEEK_SET);
    while (fgets(line, sizeof(line), f)) {
        char f_addr[256];
        int f_port;
        if (sscanf(line, "%255[^:]:%d", f_addr, &f_port) == 2) {
            if (strcmp(f_addr, addr) == 0 && f_port == port) {
                found = 1;
                break;
            }
        }
    }
    
    if (!found) {
        fprintf(f, "%s:%d\n", addr, port);
    }
    fclose(f);
}

static void list_local_servers(void) {
    char path[512];
    snprintf(path, sizeof(path), "%s/.rchat_servers", getenv("HOME"));
    
    FILE *f = fopen(path, "r");
    if (!f) {
        ui_add_message("No saved servers found. Use /connect <ip>:<port> to add one.", 0);
        return;
    }
    
    ui_add_message("Saved servers:", 0);
    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        char msg[600];
        snprintf(msg, sizeof(msg), "- %s", line);
        ui_add_message(msg, 0);
        count++;
    }
    fclose(f);
    
    if (count == 0) {
        ui_add_message("None.", 0);
    }
}

void cmd_process_input(char *cmd) {
    if (cmd[0] == '/') {
        if (strncmp(cmd, "/connect ", 9) == 0) {
            char *addr = cmd + 9;
            char *colon = strchr(addr, ':');
            int port = 4040;
            if (colon) {
                *colon = '\0';
                port = atoi(colon + 1);
            }
            save_server_to_history(addr, port);
            if (colon) *colon = ':'; 
            
            char addr_copy[256];
            strncpy(addr_copy, addr, 255);
            char *c2 = strchr(addr_copy, ':');
            if (c2) *c2 = '\0';
            
            net_connect(addr_copy, port);
        } else if (strcmp(cmd, "/list") == 0) {
            if (sock == -1) {
                list_local_servers();
            } else {
                net_send("/list\n");
            }
        } else if (strncmp(cmd, "/nickname ", 10) == 0) {
            char *new_nick = cmd + 10;
            if (strlen(new_nick) > 0) {
                strncpy(nick, new_nick, sizeof(nick) - 1);
                if (sock != -1) {
                    char req[256];
                    snprintf(req, sizeof(req), "/nick %s\n", new_nick);
                    net_send(req);
                } else {
                    ui_add_message("Nickname changed locally.", 0);
                }
            }
        } else if (strncmp(cmd, "/register ", 10) == 0) {
            if (sock == -1) { ui_add_message("Not connected!", 2); return; }
            char *pass = cmd + 10;
            unsigned char hash[crypto_generichash_BYTES];
            crypto_generichash(hash, sizeof(hash), (const unsigned char *)pass, strlen(pass), NULL, 0);
            char hex[128];
            to_hex(hash, sizeof(hash), hex);
            char req[256];
            snprintf(req, sizeof(req), "/register %s\n", hex);
            net_send(req);
        } else if (strncmp(cmd, "/login ", 7) == 0) {
            if (sock == -1) { ui_add_message("Not connected!", 2); return; }
            char *pass = cmd + 7;
            unsigned char hash[crypto_generichash_BYTES];
            crypto_generichash(hash, sizeof(hash), (const unsigned char *)pass, strlen(pass), NULL, 0);
            char hex[128];
            to_hex(hash, sizeof(hash), hex);
            char req[256];
            snprintf(req, sizeof(req), "/login %s\n", hex);
            net_send(req);
        } else if (strncmp(cmd, "/join ", 6) == 0) {
            if (sock == -1) { ui_add_message("Not connected!", 2); return; }
            char *room = cmd + 6;
            char *pass = strchr(room, ' ');
            if (pass) {
                *pass = '\0'; pass++;
                crypto_hash_password(pass, room_key);
            } else {
                crypto_hash_password("", room_key);
            }
            strncpy(current_room, room, sizeof(current_room) - 1);
            char req[256];
            snprintf(req, sizeof(req), "/join %s\n", room);
            net_send(req);
            ui_add_message("Joining room...", 0);
            ui_setup_windows(); 
        } else if (strncmp(cmd, "/ban ", 5) == 0) {
            if (sock != -1) {
                char req[256];
                snprintf(req, sizeof(req), "%s\n", cmd);
                net_send(req);
            }
        } else if (strcmp(cmd, "/exit") == 0 || strcmp(cmd, "/quit") == 0) {
            running = 0;
        } else {
            ui_add_message("Unknown command.", 2);
        }
    } else {
        if (sock == -1) { ui_add_message("Not connected! Use /connect <ip>[:port]", 2); return; }
        if (strlen(current_room) == 0) { ui_add_message("Join a room first! Use /join <room> [password]", 2); return; }
        
        unsigned char encrypted[MAX_CIPHERTEXT_LEN];
        unsigned long long encrypted_len;
        
        if (crypto_encrypt_message(room_key, cmd, encrypted, &encrypted_len) == 0) {
            char hex[MAX_CIPHERTEXT_LEN * 2 + 1];
            to_hex(encrypted, encrypted_len, hex);
            
            char req[8192];
            snprintf(req, sizeof(req), "/msg %s\n", hex);
            net_send(req);
            
            char own_msg[4100];
            snprintf(own_msg, sizeof(own_msg), "You: %s", cmd);
            ui_add_message(own_msg, 1);
        } else {
            ui_add_message("Encryption failed!", 2);
        }
    }
}
