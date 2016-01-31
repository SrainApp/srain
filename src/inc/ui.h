#ifndef __UI_H
#define __UI_H

#include <gtk/gtk.h>
#include "msg.h"

typedef struct {
    char name[20];
    GtkWidget *panel;   /* Must be a GtkBox */
} Chan;

void ui_window_init();
void ui_join_chan(const char *chan);
void ui_part_chan(Chan chan);
void ui_send_msg(const MsgSend msg);
void ui_recv_msg(const MsgRecv msg);
void ui_sys_msg(const MsgSys msg);

#endif /* __UI_H */
