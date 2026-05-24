#include "ui.h"
#include "state.h"
#include <string.h>

#define MESSAGE_HISTORY_SIZE 1000
#define INPUT_HEIGHT 3

typedef struct {
    char text[4096];
    int is_own;
} Message;

static Message message_history[MESSAGE_HISTORY_SIZE];
static int message_count = 0;

WINDOW *chat_win = NULL;
WINDOW *input_win = NULL;

void ui_init(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    nodelay(stdscr, TRUE);
    
    init_pair(1, COLOR_CYAN, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_BLACK); 
    init_pair(3, COLOR_RED, COLOR_BLACK); 
}

void ui_cleanup(void) {
    if (chat_win) delwin(chat_win);
    if (input_win) delwin(input_win);
    endwin();
}

void ui_draw_chat(void) {
    if (!chat_win) return;
    werase(chat_win);
    int max_y, max_x;
    getmaxyx(chat_win, max_y, max_x);
    (void)max_x;
    
    int start_idx = (message_count > max_y) ? message_count - max_y : 0;
    int line = 0;
    for (int i = start_idx; i < message_count; i++) {
        if (message_history[i].is_own == 1) {
            wattron(chat_win, COLOR_PAIR(2)); 
            mvwprintw(chat_win, line++, 0, "%s", message_history[i].text);
            wattroff(chat_win, COLOR_PAIR(2));
        } else if (message_history[i].is_own == 2) {
            wattron(chat_win, COLOR_PAIR(3)); 
            mvwprintw(chat_win, line++, 0, "%s", message_history[i].text);
            wattroff(chat_win, COLOR_PAIR(3));
        } else {
            wattron(chat_win, COLOR_PAIR(1));
            mvwprintw(chat_win, line++, 0, "%s", message_history[i].text);
            wattroff(chat_win, COLOR_PAIR(1));
        }
    }
    wrefresh(chat_win);
}

void ui_setup_windows(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    if (chat_win) delwin(chat_win);
    if (input_win) delwin(input_win);
    
    chat_win = newwin(max_y - INPUT_HEIGHT, max_x, 0, 0);
    scrollok(chat_win, TRUE);
    
    input_win = newwin(INPUT_HEIGHT, max_x, max_y - INPUT_HEIGHT, 0);
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, " Input (Room: %s) ", strlen(current_room) ? current_room : "None");
    wrefresh(input_win);
    ui_draw_chat();
}

void ui_add_message(const char *text, int is_own) {
    if (message_count < MESSAGE_HISTORY_SIZE) {
        strncpy(message_history[message_count].text, text, sizeof(message_history[message_count].text) - 1);
        message_history[message_count].is_own = is_own;
        message_count++;
    } else {
        for (int i = 1; i < MESSAGE_HISTORY_SIZE; i++) {
            strcpy(message_history[i-1].text, message_history[i].text);
            message_history[i-1].is_own = message_history[i].is_own;
        }
        strncpy(message_history[MESSAGE_HISTORY_SIZE-1].text, text, sizeof(message_history[MESSAGE_HISTORY_SIZE-1].text) - 1);
        message_history[MESSAGE_HISTORY_SIZE-1].is_own = is_own;
    }
}

void ui_handle_resize(void) {
    endwin();
    refresh();
    clear();
    ui_setup_windows();
}

void ui_update_input(const char *input_buf) {
    werase(input_win);
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, " Input (Room: %s) ", strlen(current_room) ? current_room : "None");
    mvwprintw(input_win, 1, 1, "> %s", input_buf);
    wrefresh(input_win);
}
