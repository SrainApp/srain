/**
 * @file srain_msg_list.c
 * @brief A auto-scrolling, dynamic loading listbox used to display messages
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-05-19
 *
 * Note: Unlike SrainUserList, SrainMagList is subclass of GtkScrolledWindow
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "ui_common.h"
#include "srain_window.h"
#include "srain_msg_list.h"
#include "srain_msg.h"
#include "snotify.h"

#include "i18n.h"
#include "markup.h"
#include "log.h"

#define MAX_MSG_COUNT 100

struct _SrainMsgList {
    GtkScrolledWindow parent;

    int vis_row_num;
    GtkListBox *list_box;
    SrainMsg *last_msg;
};

struct _SrainMsgListClass {
    GtkScrolledWindowClass parent_class;
};

G_DEFINE_TYPE(SrainMsgList, srain_msg_list, GTK_TYPE_SCROLLED_WINDOW);

static int get_list_box_length(GtkListBox *list_box){
    if (GTK_IS_LIST_BOX(list_box)){
        return g_list_length(
                gtk_container_get_children(GTK_CONTAINER(list_box)));
    }

    return 0;
}

/* scrolled_window_on_edge_overshot() and scrolled_window_on_edge_reached ()
 * are used for implement dynamic hide&load messages */

static void scrolled_window_on_edge_overshot(GtkScrolledWindow *swin,
        GtkPositionType pos, gpointer user_data){
    int i;
    SrainMsgList *list;
    GtkListBoxRow *row;

    if (pos != GTK_POS_TOP) return;

    DBG_FR("Overshot");

    list = user_data;

    for (i = MAX_MSG_COUNT - 1;
            list->vis_row_num >= 0 && i >= 0;
            list->vis_row_num--, i--){
        row = gtk_list_box_get_row_at_index(
                list->list_box, list->vis_row_num);
        DBG_FR("Hide row %p", row);
        if (GTK_IS_LIST_BOX_ROW(row)){
            gtk_widget_set_visible(GTK_WIDGET(row), TRUE);
        }
    }
}

static void scrolled_window_on_edge_reached(GtkScrolledWindow *swin,
               GtkPositionType pos, gpointer user_data){
    int len;
    SrainMsgList *list;
    GtkListBoxRow *row;

    if (pos != GTK_POS_BOTTOM) return;

    DBG_FR("Reached");
    list = user_data;

    len = get_list_box_length(list->list_box);
    for ( ;list->vis_row_num < len - MAX_MSG_COUNT;
            list->vis_row_num++){
        row = gtk_list_box_get_row_at_index(
                list->list_box, list->vis_row_num);
        DBG_FR("Hide row %p", row);
        if (GTK_IS_LIST_BOX_ROW(row)){
            gtk_widget_set_visible(GTK_WIDGET(row), FALSE);
        }
    }
}

/**
 * @brief scroll_to_bottom_idle
 *
 * @param list
 *
 * @return Always return FALSE
 *
 * This function must be called as a idle.
 *
 */
static gboolean scroll_to_bottom_idle(SrainMsgList *list){
    GtkAdjustment *adj;

    /* if this instance has been freed */
    g_return_val_if_fail(SRAIN_IS_MSG_LIST(list), FALSE);

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) -
            gtk_adjustment_get_page_size(adj));
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(list), adj);

    return FALSE;
}

/**
 * @brief smart_scroll
 *
 * @param list
 * @param force If force = 1, scroll to bottom anyway.
 *
 * If force != 1,
 * and the top-level window is visible,
 * and `list` is belonged to the current SrainChat,
 * and the value of scrolled window's adjustment (scrollbar):
 *      value + page size > max value (upper - page size),
 * scroll the list to the bottom.
 *
 */
static void smart_scroll(SrainMsgList *list, int force){
    double val;
    double max_val;
    double page_size;
    GtkAdjustment *adj;
    SrainWindow *win;
    SrainChat *chat;

    win = SRAIN_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(list)));
    if (!SRAIN_IS_WINDOW(win)){
        ERR_FR("Top level widget is not SrainWindow");
        return;
    }

    chat = srain_window_get_cur_chat(win);
    if (!SRAIN_IS_CHAT(chat)){
        ERR_FR("Current chat is no a SrainChat");
        return;
    }

    if (force){
        gdk_threads_add_idle((GSourceFunc)scroll_to_bottom_idle, list);
        return;
    }

    if (gtk_widget_get_visible(GTK_WIDGET(win))
            && srain_chat_get_msg_list(chat) == list){

        adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
        val = gtk_adjustment_get_value(adj);
        page_size = gtk_adjustment_get_page_size(adj);
        max_val = gtk_adjustment_get_upper(adj) - page_size;

        if (val + page_size > max_val){
            gdk_threads_add_idle((GSourceFunc)scroll_to_bottom_idle, list);
        }
    }
}

void srain_msg_list_scroll_up(SrainMsgList *list, double step){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) - step);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(list), adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

void srain_msg_list_scroll_down(SrainMsgList *list, double step){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) + step);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(list), adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

static void srain_msg_list_init(SrainMsgList *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    self->vis_row_num = 0;
    g_signal_connect(self, "edge-overshot",
            G_CALLBACK(scrolled_window_on_edge_overshot), self);
    g_signal_connect(self, "edge-reached",
            G_CALLBACK(scrolled_window_on_edge_reached), self);
}

static void srain_msg_list_class_init(SrainMsgListClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/msg_list.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, list_box);
}

SrainMsgList* srain_msg_list_new(void){
    return g_object_new(SRAIN_TYPE_MSG_LIST, NULL);
}

void srain_msg_list_sys_msg_add(SrainMsgList *list, const char *msg,
        SysMsgType type, SrainMsgFlag flag){
    GtkListBoxRow *row;
    SrainSysMsg *smsg;

    /* Do not merge sys message (看起来不好看 :-|)
    if (list->last_msg
            && SRAIN_IS_SYS_MSG(list->last_msg)
            && SRAIN_SYS_MSG(list->last_msg)->type != SYS_MSG_ACTION){
        if (srain_msg_append_msg(list->last_msg, msg, flag) == 0){
            smart_scroll(list, 0);
            return;
        }
    }
    */

    smsg = srain_sys_msg_new(msg, type, flag);
    row = gtk_list_box_add_unfocusable_row(list->list_box, GTK_WIDGET(smsg));

    // TODO: 需要 Action message 和 Error message 的区分
    if (flag & SRAIN_MSG_MENTIONED){
        gtk_widget_set_name(GTK_WIDGET(row), "mentioned_msg");
        if (!srain_window_is_active(srain_win) && type == SYS_MSG_ACTION){
            snotify_notify( _("Action"), msg, "srain");
            srain_window_tray_icon_stress(srain_win, 1);
        }
    }

    if (type == SYS_MSG_ERROR) {
        if (!srain_window_is_active(srain_win)){
            snotify_notify(_("Error"), msg, "srain-red");
            srain_window_tray_icon_stress(srain_win, 1);
        }
    }

    list->last_msg = SRAIN_MSG(smsg);

    smart_scroll(list, 0);
}


void srain_msg_list_send_msg_add(SrainMsgList *list, const char *msg, SrainMsgFlag flag){
    SrainSendMsg *smsg;

    if (list->last_msg && SRAIN_IS_SEND_MSG(list->last_msg)){
        if (srain_msg_append_msg(list->last_msg, msg, flag) == 0){
            smart_scroll(list, 1);
            return;
        }
    }

    smsg = srain_send_msg_new(msg, flag);
    gtk_list_box_add_unfocusable_row(list->list_box, GTK_WIDGET(smsg));

    list->last_msg = SRAIN_MSG(smsg);

    smart_scroll(list, 1);
}

/* Add a SrainRecvMsg into SrainMsgList.  */
void srain_msg_list_recv_msg_add(SrainMsgList *list, const char *nick,
        const char *id, const char *msg, SrainMsgFlag flag){
    GtkListBoxRow *row;
    SrainRecvMsg *smsg;

    if (list->last_msg && SRAIN_IS_RECV_MSG(list->last_msg)){
        const char *old_nick = gtk_label_get_text(
                SRAIN_RECV_MSG(list->last_msg)->nick_label);
        if (strcasecmp(nick, old_nick) == 0) {
            if (srain_msg_append_msg(list->last_msg, msg, flag) == 0){
                smart_scroll(list, 1);
                return;
            }
        }
    }

    smsg = srain_recv_msg_new(nick, id, msg, flag);
    row = gtk_list_box_add_unfocusable_row(list->list_box, GTK_WIDGET(smsg));

    if (flag & SRAIN_MSG_MENTIONED){
        gtk_widget_set_name(GTK_WIDGET(row), "mentioned_msg");
        if (!srain_window_is_active(srain_win)) {
            snotify_notify(nick, msg, "srain");
            srain_window_tray_icon_stress(srain_win, 1);
        }
    }

    list->last_msg = SRAIN_MSG(smsg);

    smart_scroll(list, 0);
}
