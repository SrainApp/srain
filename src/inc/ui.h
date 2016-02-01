#ifndef __UI_H
#define __UI_H

#include <gtk/gtk.h>
#include "msg.h"

typedef struct {
    char name[20];
    char *topic;
} Chat;

void ui_window_init();
void ui_new_chat(const Chat chat);
void ui_rm_chat(const char *chat_name);
void ui_send_msg(const MsgSend msg);
void ui_recv_msg(const MsgRecv msg);
void ui_sys_msg(const MsgSys msg);

#endif /* __UI_H */
