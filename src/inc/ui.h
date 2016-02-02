#ifndef __UI_H
#define __UI_H

#include <gtk/gtk.h>
#include "msg.h"

void ui_window_init();
int ui_new_chat(const char *name, const char *topic);
int ui_rm_chat(const char *name);
void ui_send_msg(const MsgSend msg);
void ui_recv_msg(const MsgRecv msg);
void ui_sys_msg(const MsgSys msg);

#endif /* __UI_H */
