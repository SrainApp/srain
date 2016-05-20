/**
 * @file srain_msg_list.c
 * @brief A auto-scrolling, dynamic loading listbox used to display messages
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-05-19
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "ui_common.h"
#include "srain_msg_list.h"
#include "srain_msg.h"

#include "irc_magic.h"

#include "markup.h"
#include "log.h"

struct _SrainMsgList {
    GtkScrolledWindow parent;
    GtkListBox *list;
    GtkWidget *last_msg;
};

struct _SrainMsgListClass {
    GtkScrolledWindowClass parent_class;
};

G_DEFINE_TYPE(SrainMsgList, srain_msg_list, GTK_TYPE_SCROLLED_WINDOW);

static void srain_msg_list_scroll_up(SrainMsgList *list){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) - 30);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(list), adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

static void srain_msg_list_scroll_down(SrainMsgList *list){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) + 30);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(list), adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

static gboolean scroll_to_bottom(SrainMsgList *list){
    GtkAdjustment *adj;
    double val;
    double max_val;

    /* if this instance has been freed */
    if (!SRAIN_IS_MSG_LIST(list)) return FALSE;

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) -
            gtk_adjustment_get_page_size(adj));
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(list), adj);

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    val = gtk_adjustment_get_value(adj);
    max_val = gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj);
    // LOG_FR("cur val: %f, max val %f", val, max_val);

    if (max_val - val > 10) {
        LOG_FR("retry");
        return TRUE;
    }

    return FALSE;
}

static void srain_msg_list_init(SrainMsgList *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_msg_list_class_init(SrainMsgListClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/msg_list.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, list);
}

SrainMsgList* srain_msg_list_new(void){
    return g_object_new(SRAIN_TYPE_MSG_LIST, NULL);
}

void srain_msg_list_sys_msg_add(SrainMsgList *list, const char *msg, SysMsgType type){
    int to_bottom;
    SrainSysMsg *smsg;

    smsg = srain_sys_msg_new(msg, type);

    gtk_list_box_add_unfocusable_row(list->list, GTK_WIDGET(smsg));

    list->last_msg = GTK_WIDGET(smsg);
}


void srain_msg_list_send_msg_add(SrainMsgList *list, const char *msg){
    SrainSendMsg *smsg;

    smsg = srain_send_msg_new(msg);
    gtk_list_box_add_unfocusable_row(list->list, GTK_WIDGET(smsg));

    list->last_msg = GTK_WIDGET(smsg);

    gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, list);
}

void _srain_msg_list_recv_msg_add(SrainMsgList *list, const char *nick,
        const char *id, const char *msg){
    SrainRecvMsg *smsg;

    smsg = srain_recv_msg_new(nick, id, msg);
    gtk_list_box_add_unfocusable_row(list->list, GTK_WIDGET(smsg));

    list->last_msg = GTK_WIDGET(smsg);
}

/* add a SrainRecvMsg into SrainMsgList, if its time is same to the last msg, combine them */
void srain_msg_list_recv_msg_add(SrainMsgList *list, const char *nick,
        const char *id, const char *msg){
    int to_bottom;
    char timestr[32];
    const char *old_msg;
    const char *old_timestr;
    const char *old_nick;
    GString *new_msg;
    GString *markuped_msg;
    SrainRecvMsg *last_recv_msg;

    get_cur_time(timestr);

    if (list->last_msg && SRAIN_IS_RECV_MSG(list->last_msg)){
        last_recv_msg = SRAIN_RECV_MSG(list->last_msg);
        old_msg = gtk_label_get_text(last_recv_msg->msg_label);
        old_timestr = gtk_label_get_text(last_recv_msg->time_label);
        old_nick = gtk_label_get_text(last_recv_msg->nick_label);

        /* a message that
         *  - send by the same people
         *  - send in same minute
         *  - less then 512 char
         * can be combine
         */
        if (strncmp(timestr, old_timestr, 32) == 0
                && strncmp(nick, old_nick, NICK_LEN) == 0
                && strlen(old_msg) < 512){
            new_msg = g_string_new(old_msg);
            g_string_append(new_msg, "\n");
            g_string_append(new_msg, msg);

            markuped_msg = markup(new_msg->str, NULL);
            if (markuped_msg){
                gtk_label_set_markup(last_recv_msg->msg_label, markuped_msg->str);
                g_string_free(markuped_msg, TRUE);
            } else {
                gtk_label_set_text(last_recv_msg->msg_label, new_msg->str);
            }

            g_string_free(new_msg, TRUE);
            if (to_bottom) gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, list);

            return;
        }
    }

    _srain_msg_list_recv_msg_add(list, nick, id, msg);
}

