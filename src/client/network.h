#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

void net_connect(const char *host, int port);
void net_disconnect(void);
void net_send(const char *msg);
void net_process_incoming(void);

#endif
