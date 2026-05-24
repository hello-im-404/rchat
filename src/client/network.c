#include "network.h"
#include "state.h"
#include "ui.h"
#include "../common/crypto.h"
#include "../common/utils.h"
#include "../common/buffer.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

static ByteBuffer net_buf;

static void set_nonblocking(int sock_fd) {
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
}

void net_connect(const char *host, int port) {
    struct hostent *he;
    struct sockaddr_in server_addr;

    if ((he = gethostbyname(host)) == NULL) {
        ui_add_message("Failed to resolve hostname.", 2);
        return;
    }
    if (sock != -1) close(sock);
    buffer_free(&net_buf);
    buffer_init(&net_buf, 4096);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ui_add_message("Connection failed!", 2);
        close(sock);
        sock = -1;
        return;
    }
    set_nonblocking(sock);
    send(sock, nick, strlen(nick), 0);
    ui_add_message("Connected successfully! Use /join <room> [password] to enter a room.", 0);
}

void net_disconnect(void) {
    if (sock != -1) {
        close(sock);
        sock = -1;
        buffer_free(&net_buf);
    }
}

void net_send(const char *msg) {
    if (sock != -1) {
        send(sock, msg, strlen(msg), 0);
    }
}

void net_process_incoming(void) {
    char buf[8192];
    int valread = recv(sock, buf, sizeof(buf), 0);
    if (valread <= 0) {
        ui_add_message("Server disconnected!", 2);
        net_disconnect();
    } else {
        buffer_append(&net_buf, buf, valread);
        char *line = NULL;
        while (buffer_extract_line(&net_buf, &line)) {
            if (strncmp(line, "/msg ", 5) == 0) {
                char *sender = line + 5;
                char *hex = strchr(sender, ' ');
                if (hex) {
                    *hex = '\0'; hex++;
                    unsigned char ciphertext[16384];
                    int ciphertext_len = from_hex(hex, ciphertext);
                    
                    if (ciphertext_len > 0) {
                        char decrypted[16384];
                        int dec_res = crypto_decrypt_message(room_key, ciphertext, ciphertext_len, decrypted);
                        if (dec_res == 0) {
                            char display[17000];
                            snprintf(display, sizeof(display), "[%s]: %s", sender, decrypted);
                            ui_add_message(display, 0);
                        } else if (dec_res == -2) {
                            ui_add_message("[Replay attack intercepted and dropped.]", 2);
                        } else {
                            ui_add_message("[Decryption failed. Invalid password or corrupted message.]", 2);
                        }
                    }
                }
            } else {
                ui_add_message(line, 0);
            }
            free(line);
        }
        ui_draw_chat();
    }
}
