/**
 * @file srain_msg_list.c
 * @brief A auto-scrolling, dynamic loading listbox used to display messages
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-05-19
 *
 * Note: Unlike SrainUserList, SrainMagList is subclass of GtkScrolledWindow
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "ui_common.h"
#include "ui_hdr.h"
#include "srain_msg_list.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_msg.h"

// TODO
#include "srv_session.h"

#include "i18n.h"
#include "markup.h"
#include "log.h"

#define MAX_MSG_COUNT 100

struct _SrainMsgList {
    GtkScrolledWindow parent;

    int vis_row_num;
    GtkListBox *list_box;
    GtkWidget *last_msg;

    // nick_menu
    GtkMenu *nick_menu;
    GtkMenuItem *whois_menu_item;
    GtkMenuItem *kick_menu_item;
    GtkMenuItem *chat_menu_item;
    GtkMenuItem *invite_menu_item;
};

struct _SrainMsgListClass {
    GtkScrolledWindowClass parent_class;
};

G_DEFINE_TYPE(SrainMsgList, srain_msg_list, GTK_TYPE_SCROLLED_WINDOW);

/* The right-clicked widgets */
static GtkButton *clicked_button = NULL;
static GtkLabel *clicked_label = NULL;

/**
 * @brief Get the selected text (utf-8 supported) of `clicked_label`,
 *      if no text was selected, return all of the text in this label,
 *      if there is any '\n'(newline) in the text, strip it.
 *
 * @return A allocated (char *), it should be freed by `free()`
 */
static char* clicked_label_get_selection(){
    int i;
    int start, end;
    const char *msg;
    char *sel_msg;
    if (!clicked_label) return NULL;

    msg = gtk_label_get_text(clicked_label);

    if (gtk_label_get_selection_bounds(clicked_label, &start, &end)){
        sel_msg = g_utf8_substring(msg, start, end);
    } else {
        sel_msg = strdup(msg);
    }

    /* Strip '\n' */
    for (i = 0; i < strlen(sel_msg); i++){
        if (sel_msg[i] == '\n')
            sel_msg[i] = ' ';
    }

    return sel_msg;
}

static void nick_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    const char *nick;
    GList *list;
    GString *cmd;

    if (!clicked_button) return;
    list = gtk_container_get_children(GTK_CONTAINER(clicked_button));
    nick = gtk_label_get_text(list->data);

    cmd = g_string_new("");

    if (strcmp(gtk_widget_get_name(widget), "whois_menu_item") == 0){
        g_string_printf(cmd, "/whois %s", nick);
    }
    else if (strcmp(gtk_widget_get_name(widget), "kick_menu_item") == 0){
        g_string_printf(cmd, "/kick %s", nick);
    }
    else if (strcmp(gtk_widget_get_name(widget), "chat_menu_item") == 0){
        g_string_printf(cmd, "/query %s", nick);
    }
    else if (strcmp(gtk_widget_get_name(widget), "invite_submenu_item") == 0){
        g_string_printf(cmd, "/invite %s %s", nick,
                gtk_menu_item_get_label(GTK_MENU_ITEM(widget)));
    }
    else {
        ERR_FR("Unknown menu item: %s", gtk_widget_get_name(widget));
        g_string_free(cmd, TRUE);

        return;
    }

    ui_hdr_srv_cmd(srain_window_get_cur_chan(srain_win), cmd->str);

    g_string_free(cmd, TRUE);
}

static void quote_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char *quote_msg;
    GString *str;

    if ((quote_msg = clicked_label_get_selection()) == NULL) return;

    str = g_string_new("");
    g_string_printf(str, _("Quote { %s } "), quote_msg);
    g_free(quote_msg);

    srain_chan_insert_text(srain_window_get_cur_chan(srain_win), str->str, 0);
    g_string_free(str, TRUE);
}

static void froward_submenu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char *fwd_msg;
    const char *srv_name;
    GString *str;
    SrainChan *chan;

    if ((fwd_msg = clicked_label_get_selection()) == NULL) return;

    srv_name = srain_chan_get_server_name(srain_window_get_cur_chan(srain_win));
    chan = srain_window_get_chan_by_name(srain_win, srv_name,
            gtk_menu_item_get_label(GTK_MENU_ITEM(widget)));

    str = g_string_new("");
    g_string_printf(str, _("%s <Forward from %s>"), fwd_msg,
            srain_chan_get_chan_name(srain_window_get_cur_chan(srain_win)));
    g_free(fwd_msg);

    // TODO: error message when failed to send?
    srain_msg_list_send_msg_add(srain_chan_get_msg_list(chan), str->str);
    ui_hdr_srv_send(chan, str->str);

    g_string_free(str, TRUE);
}

static gboolean nick_button_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    GList *chans;
    GtkMenuItem *item;
    SrainChan *chan;
    SrainMsgList *list;

    list = SRAIN_MSG_LIST(user_data);
    if (event->button == 3){
        gtk_menu_popup(list->nick_menu, NULL, NULL, NULL, NULL,
                event->button, event->time);

        /* If SrainRecvMsg->nick_button was right-clicked,
         * set the global varible `clicked_button` */
        clicked_button = GTK_BUTTON(widget);

        /* Create subitem of invite_menu_item */
        // FIXME: will these menus auto freed?
        GtkMenu *invite_submenu = GTK_MENU(gtk_menu_new());

        gtk_menu_item_set_submenu(list->invite_menu_item, GTK_WIDGET(invite_submenu));

        chan = srain_window_get_cur_chan(srain_win);

        chans = srain_window_get_chans_by_srv_name(srain_win,
                srain_chan_get_server_name(chan));
        while (chans){
            item  = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                        srain_chan_get_chan_name(chans->data)));
            gtk_widget_show(GTK_WIDGET(item));
            gtk_widget_set_name(GTK_WIDGET(item), "invite_submenu_item");
            g_signal_connect(item, "activate",
                    G_CALLBACK(nick_menu_item_on_activate), NULL);
            gtk_menu_shell_append(GTK_MENU_SHELL(invite_submenu), GTK_WIDGET(item));

            chans = g_list_next(chans);
        }

        g_list_free(chans);

        return TRUE;
    }
    return FALSE;
}

static void msg_label_on_popup(GtkLabel *label, GtkMenu *menu,
        gpointer user_data){
    GtkMenuItem *quote_menu_item;
    GtkMenuItem *forward_menu_item;

    /* If SrainRecvMsg->msg_label was right-clicked,
     * set the global varible `clicked_label` */
    clicked_label = label;

    /* Create menuitem forward_menu_item */
    forward_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Forward to...")));
    gtk_widget_show(GTK_WIDGET(forward_menu_item));
    gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), GTK_WIDGET(forward_menu_item));

    /* Create menuitem quote_menu_item */
    quote_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Quote")));
    gtk_widget_show(GTK_WIDGET(quote_menu_item));
    g_signal_connect(quote_menu_item, "activate",
            G_CALLBACK(quote_menu_item_on_activate), NULL);
    gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), GTK_WIDGET(quote_menu_item));

    /* Create submenu of forward_menu_item */
    // FIXME: will these menus auto freed?
    GList *chans;
    GtkMenuItem *item;
    GtkMenu *forward_submenu = GTK_MENU(gtk_menu_new());
    SrainChan *chan;

    gtk_menu_item_set_submenu(forward_menu_item, GTK_WIDGET(forward_submenu));

    chan = srain_window_get_cur_chan(srain_win);

    chans = srain_window_get_chans_by_srv_name(srain_win,
            srain_chan_get_server_name(chan));
    while (chans){
        item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                    srain_chan_get_chan_name(chans->data)));
        gtk_widget_show(GTK_WIDGET(item));
        g_signal_connect(item, "activate",
                G_CALLBACK(froward_submenu_item_on_activate), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(forward_submenu), GTK_WIDGET(item));

        chans = g_list_next(chans);
    }

    g_list_free(chans);

}

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
 * @brief scroll_to_bottom
 *
 * @param list
 *
 * @return Always return FALSE
 *
 * This function must be called as a idle.
 *
 */
static gboolean scroll_to_bottom(SrainMsgList *list){
    GtkAdjustment *adj;

    /* if this instance has been freed */
    if (!SRAIN_IS_MSG_LIST(list)) return FALSE;

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
 * and `list` is belonged to the current SrainChan,
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
    SrainChan *chan;

    win = SRAIN_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(list)));
    if (!SRAIN_IS_WINDOW(win)){
        ERR_FR("Top level widget is not SrainWindow");
        return;
    }

    chan = srain_window_get_cur_chan(win);
    if (!SRAIN_IS_CHAN(chan)){
        ERR_FR("Current chan is no a SrainChan");
        return;
    }

    if (force){
        gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, list);
        return;
    }

    if (gtk_widget_get_visible(GTK_WIDGET(win))
            && srain_chan_get_msg_list(chan) == list){

        adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(list));
        val = gtk_adjustment_get_value(adj);
        page_size = gtk_adjustment_get_page_size(adj);
        max_val = gtk_adjustment_get_upper(adj) - page_size;

        if (val + page_size > max_val){
            gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, list);
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
    g_signal_connect(self->whois_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), NULL);
    g_signal_connect(self->kick_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), NULL);
    g_signal_connect(self->chat_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), NULL);
    // g_signal_connect(self->invite_menu_item, "activate",
            // G_CALLBACK(nick_menu_item_on_activate), NULL);
}

static void srain_msg_list_class_init(SrainMsgListClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/msg_list.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, list_box);

    // nick_menu
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, nick_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, whois_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, kick_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, chat_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainMsgList, invite_menu_item);
}

SrainMsgList* srain_msg_list_new(void){
    return g_object_new(SRAIN_TYPE_MSG_LIST, NULL);
}

void srain_msg_list_sys_msg_add(SrainMsgList *list, const char *msg, SysMsgType type){
    SrainSysMsg *smsg;

    smsg = srain_sys_msg_new(msg, type);
    gtk_list_box_add_unfocusable_row(list->list_box, GTK_WIDGET(smsg));

    list->last_msg = GTK_WIDGET(smsg);

    smart_scroll(list, 0);
}


void srain_msg_list_send_msg_add(SrainMsgList *list, const char *msg){
    SrainSendMsg *smsg;

    smsg = srain_send_msg_new(msg);
    gtk_list_box_add_unfocusable_row(list->list_box, GTK_WIDGET(smsg));

    list->last_msg = GTK_WIDGET(smsg);

    smart_scroll(list, 1);
}

void _srain_msg_list_recv_msg_add(SrainMsgList *list, const char *nick,
        const char *id, const char *msg){
    SrainRecvMsg *smsg;

    smsg = srain_recv_msg_new(nick, id, msg);
    gtk_list_box_add_unfocusable_row(list->list_box, GTK_WIDGET(smsg));

    g_signal_connect(smsg->nick_button, "button-press-event",
            G_CALLBACK(nick_button_on_popup), list);
    g_signal_connect(smsg->msg_label, "populate-popup",
            G_CALLBACK(msg_label_on_popup), list);

    list->last_msg = GTK_WIDGET(smsg);

    smart_scroll(list, 0);
}

/* Add a SrainRecvMsg into SrainMsgList.
 * If its time is same to the last msg, combine them.
 */
void srain_msg_list_recv_msg_add(SrainMsgList *list, const char *nick,
        const char *id, const char *msg){
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

        /* A message that
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
            smart_scroll(list, 0);

            return;
        }
    }

    _srain_msg_list_recv_msg_add(list, nick, id, msg);
}

