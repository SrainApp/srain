#ifndef __UI_H
#define __UI_H

#include <gtk/gtk.h>
#include "msg.h"

void ui_window_init();

int ui_chat_add(const char *name, const char *topic);
int ui_chat_rm(const char *name);
int ui_chat_set_topic(const char *name, const char *topic);
int ui_online_list_add(const char *chat_name, const char *nick);
int ui_online_list_rm(const char *chat_name, const char *nick);

void ui_msg_init();
int ui_msg_send(const MsgSend msg);
gboolean ui_msg_recv(MsgRecv *msg);
int ui_msg_sys(const MsgSys msg);

#endif /* __UI_H */
