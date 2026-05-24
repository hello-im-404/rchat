#ifndef SERVER_ROOMS_H
#define SERVER_ROOMS_H

void rooms_handle_join(int client_index, const char *room);
void rooms_handle_ban(int client_index, const char *target_nick);
void rooms_handle_msg(int client_index, const char *msg);

#endif
