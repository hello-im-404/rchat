#ifndef CLIENT_UI_H
#define CLIENT_UI_H

void ui_init(void);
void ui_cleanup(void);
void ui_setup_windows(void);
void ui_add_message(const char *text, int is_own);
void ui_draw_chat(void);
void ui_handle_resize(void);
void ui_update_input(const char *input_buf);

#endif
