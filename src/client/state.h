#ifndef CLIENT_STATE_H
#define CLIENT_STATE_H
#include <ncurses.h>
#include <sodium.h>

extern int sock;
extern char nick[64];
extern char current_room[64];
extern unsigned char room_key[crypto_secretbox_KEYBYTES];
extern int running;
extern WINDOW *chat_win;
extern WINDOW *input_win;

#endif
