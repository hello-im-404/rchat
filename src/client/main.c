#include "state.h"
#include "ui.h"
#include "network.h"
#include "commands.h"
#include "../common/crypto.h"
#include <string.h>
#include <errno.h>
#include <sys/select.h>

int sock = -1;
char nick[64];
char current_room[64] = "";
unsigned char room_key[crypto_secretbox_KEYBYTES];
int running = 1;

int main(void) {
    crypto_init();
    ui_init();
    ui_setup_windows();

    echo();
    mvwprintw(input_win, 1, 1, "Enter your nickname: ");
    wrefresh(input_win);
    nodelay(input_win, FALSE);
    wgetnstr(input_win, nick, sizeof(nick) - 1);
    if(strlen(nick) == 0) strcpy(nick, "Anonymous");
    noecho();
    nodelay(input_win, TRUE);
    keypad(input_win, TRUE);

    ui_add_message("  _____   _____ _           _   ", 0);
    ui_add_message(" |  __ \\ / ____| |         | |  ", 0);
    ui_add_message(" | |__) | |    | |__   __ _| |_ ", 0);
    ui_add_message(" |  _  /| |    | '_ \\ / _` | __|", 0);
    ui_add_message(" | | \\ \\| |____| | | | (_| | |_ ", 0);
    ui_add_message(" |_|  \\_\\_____|_| |_|\\__,_|\\__|", 0);
    ui_add_message("                                ", 0);
    ui_add_message("Welcome! Use /connect <ip>[:port] to connect to a server.", 0);
    ui_draw_chat();
    ui_update_input("");

    fd_set readfds;
    char input_buf[1024] = {0};
    int input_len = 0;

    while (running) {
        FD_ZERO(&readfds);
        if (sock != -1) FD_SET(sock, &readfds);
        
        int max_fd = sock;

        struct timeval tv = {0, 50000}; 
        int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0 && errno != EINTR) {
            break;
        }

        if (sock != -1 && FD_ISSET(sock, &readfds)) {
            net_process_incoming();
        }

        int ch;
        while ((ch = wgetch(input_win)) != ERR) {
            if (ch == KEY_RESIZE) {
                ui_handle_resize();
                ui_update_input(input_buf);
            } else if (ch == '\n' || ch == '\r') {
                input_buf[input_len] = '\0';
                if (input_len > 0) {
                    cmd_process_input(input_buf);
                    input_len = 0;
                    input_buf[0] = '\0';
                }
                ui_update_input("");
                ui_draw_chat();
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                if (input_len > 0) {
                    input_len--;
                    input_buf[input_len] = '\0';
                    ui_update_input(input_buf);
                }
            } else if (ch >= 32 && ch <= 126 && input_len < sizeof(input_buf) - 1) {
                input_buf[input_len++] = ch;
                input_buf[input_len] = '\0';
                ui_update_input(input_buf);
            }
        }
    }

    net_disconnect();
    ui_cleanup();
    return 0;
}
