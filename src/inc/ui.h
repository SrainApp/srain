#ifndef __UI_H
#define __UI_H

#include <gtk/gtk.h>

typedef struct {
    char name[20];
    GtkWidget *panel;   /* Must be a GtkBox */
} Chan;

void ui_window_init();
void ui_join_chan(const char *chan);
void ui_part_chan(Chan chan);
void ui_send_msg(const char *msg);
void ui_recv_msg(const char *msg);
void ui_sys_msg(const char *msg);

#endif /* __UI_H */
