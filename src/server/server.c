#include "server.h"
#include "clients.h"
#include "rooms.h"
#include "auth.h"
#include "../common/buffer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_EVENTS 64

static void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

static void send_to_client(int sock, const char *msg) {
    char buf[8192];
    snprintf(buf, sizeof(buf), "%s\n", msg);
    send(sock, buf, strlen(buf), 0);
}

static void handle_client_data(int index, char *buffer) {
    if (strncmp(buffer, "/login ", 7) == 0) {
        char *hash = buffer + 7;
        if (auth_login(clients[index].nick, hash)) {
            clients[index].needs_auth = 0;
            send_to_client(clients[index].socket, "[SERVER] Authentication successful. Welcome back!");
        } else {
            send_to_client(clients[index].socket, "[SERVER] Invalid password!");
        }
        return;
    } else if (strncmp(buffer, "/register ", 10) == 0) {
        char *hash = buffer + 10;
        if (auth_register(clients[index].nick, hash)) {
            clients[index].needs_auth = 0;
            send_to_client(clients[index].socket, "[SERVER] Nickname registered successfully!");
        } else {
            send_to_client(clients[index].socket, "[SERVER] Nickname is already registered.");
        }
        return;
    }

    if (clients[index].needs_auth) {
        send_to_client(clients[index].socket, "[SERVER] You must /login <password> first!");
        return;
    }

    if (strncmp(buffer, "/join ", 6) == 0) {
        char *room = buffer + 6;
        rooms_handle_join(index, room);
    } else if (strncmp(buffer, "/ban ", 5) == 0) {
        char *target = buffer + 5;
        rooms_handle_ban(index, target);
    } else if (strncmp(buffer, "/msg ", 5) == 0) {
        char *msg = buffer + 5;
        rooms_handle_msg(index, msg);
    } else if (strncmp(buffer, "/nick ", 6) == 0) {
        char *new_nick = buffer + 6;
        strncpy(clients[index].nick, new_nick, 63);
        if (auth_is_registered(new_nick)) {
            clients[index].needs_auth = 1;
            send_to_client(clients[index].socket, "[SERVER] Nickname changed. This nickname is registered. Please /login <password>");
        } else {
            send_to_client(clients[index].socket, "[SERVER] Nickname changed successfully.");
        }
    } else if (strcmp(buffer, "/list") == 0) {
        char list_buf[8192] = "[SERVER] Active rooms:\n";
        char seen_rooms[MAX_CLIENTS][64] = {0};
        int seen_count = 0;
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0 && strlen(clients[i].room) > 0) {
                int already_seen = 0;
                for (int j = 0; j < seen_count; j++) {
                    if (strcmp(seen_rooms[j], clients[i].room) == 0) {
                        already_seen = 1;
                        break;
                    }
                }
                if (!already_seen) {
                    strncpy(seen_rooms[seen_count++], clients[i].room, 63);
                    int users_in_room = 0;
                    for (int k = 0; k < MAX_CLIENTS; k++) {
                        if (clients[k].socket > 0 && strcmp(clients[k].room, clients[i].room) == 0) {
                            users_in_room++;
                        }
                    }
                    char room_info[128];
                    snprintf(room_info, sizeof(room_info), "- %s (%d users)\n", clients[i].room, users_in_room);
                    strncat(list_buf, room_info, sizeof(list_buf) - strlen(list_buf) - 1);
                }
            }
        }
        if (seen_count == 0) {
            strncat(list_buf, "None.", sizeof(list_buf) - strlen(list_buf) - 1);
        }
        send_to_client(clients[index].socket, list_buf);
    }
}

void server_start(int port) {
    auth_init();
    clients_init();

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) { perror("socket"); exit(EXIT_FAILURE); }
    
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(serverfd);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }

    if (listen(serverfd, SOMAXCONN) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) { perror("epoll_create1"); exit(EXIT_FAILURE); }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = serverfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverfd, &ev) < 0) {
        perror("epoll_ctl: serverfd"); exit(EXIT_FAILURE);
    }

    printf("Server (epoll mode) started on port %d.\n", port);
    
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == serverfd) {
                while (1) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int conn_sock = accept(serverfd, (struct sockaddr *)&client_addr, &client_len);
                    if (conn_sock < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        break;
                    }
                    
                    char *ip = inet_ntoa(client_addr.sin_addr);
                    if (clients_is_banned(ip)) {
                        close(conn_sock);
                        continue;
                    }

                    char nick[64] = {0};
                    int nick_len = recv(conn_sock, nick, sizeof(nick) - 1, 0);
                    if (nick_len > 0) {
                        nick[nick_len] = '\0';
                        set_nonblocking(conn_sock);
                        clients_add(conn_sock, ip, nick);
                        
                        int new_idx = clients_find_by_socket(conn_sock);
                        if (auth_is_registered(nick)) {
                            clients[new_idx].needs_auth = 1;
                            send_to_client(conn_sock, "[SERVER] This nickname is registered. Please /login <password>");
                        } else {
                            send_to_client(conn_sock, "[SERVER] Your nickname is not registered. You can secure it by typing: /register <password>");
                        }
                        
                        ev.events = EPOLLIN | EPOLLET; 
                        ev.data.fd = conn_sock;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &ev) < 0) {
                            close(conn_sock);
                        }
                    } else {
                        close(conn_sock);
                    }
                }
            } else {
                int client_sock = events[n].data.fd;
                int client_idx = clients_find_by_socket(client_sock);
                if (client_idx == -1) {
                    close(client_sock);
                    continue;
                }

                while (1) {
                    char buf[4096];
                    int valread = recv(client_sock, buf, sizeof(buf), 0);
                    if (valread < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break; 
                        } else {
                            clients_remove(client_idx); 
                            break;
                        }
                    } else if (valread == 0) {
                        clients_remove(client_idx); 
                        break;
                    } else {
                        buffer_append(&clients[client_idx].buf, buf, valread);
                        char *line = NULL;
                        while (buffer_extract_line(&clients[client_idx].buf, &line)) {
                            handle_client_data(client_idx, line);
                            free(line);
                        }
                    }
                }
            }
        }
    }
    close(epoll_fd);
}
