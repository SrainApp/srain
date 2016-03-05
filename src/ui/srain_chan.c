/**
 * @file srain_chan.c
 * @brief
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include "ui.h"
#include "ui_common.h"
#include "theme.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_msg.h"
#include "srain_detail_dialog.h"
#include "srain.h"
#include "log.h"
#include "irc.h"

struct _SrainChan {
    GtkBox parent;
    GtkLabel* name_label;
    GtkLabel *topic_label;
    GtkScrolledWindow *msg_scrolledwindow;
    GtkBox *msg_box;
    GtkMenu *msg_menu;
    GtkRevealer *revealer;
    GtkButton *onlinelist_button;
    GtkListBox *online_listbox;
    GtkButton *send_button;
    GtkEntry *input_entry;
    GtkWidget *last_msg;
};

struct _SrainChanClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChan, srain_chan, GTK_TYPE_BOX);

// TODO not work
static void scroll_to_bottom(SrainChan *chan){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(chan->msg_scrolledwindow);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) -
            gtk_adjustment_get_page_size(adj));
    gtk_scrolled_window_set_vadjustment(chan->msg_scrolledwindow, adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

static void onlinelist_button_on_click(GtkWidget *widget, gpointer *user_data){
    gboolean is_show;
    GtkImage *image;
    GtkRevealer *revealer;

    image = GTK_IMAGE(gtk_button_get_image(GTK_BUTTON(widget)));
    revealer = GTK_REVEALER(user_data);
    is_show = gtk_revealer_get_reveal_child(revealer);

    gtk_revealer_set_reveal_child(revealer, !is_show);
    gtk_image_set_from_icon_name(image, !is_show ? "go-next":"go-previous", GTK_ICON_SIZE_BUTTON);
}

static gint online_listbox_on_dbclick(GtkWidget *widget, GdkEventButton *event){
    const char *nick;
    GtkLabel *label;
    GtkListBoxRow *row;
    SrainWindow *toplevel;
    SrainDetailDialog *dlg;

    toplevel = SRAIN_WINDOW(gtk_widget_get_toplevel(widget));
    if(event->button == 1 && event->type == GDK_2BUTTON_PRESS){
        row = gtk_list_box_get_selected_row(GTK_LIST_BOX(widget));
        if (row){
            label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row)));
            nick = gtk_label_get_text(label);

            dlg = srain_detail_dialog_new(toplevel, nick, "");
            gtk_window_present(GTK_WINDOW(dlg));
        }
    }
    return FALSE;
}

static void on_send(SrainChan *chan){
    int res;
    const char *input;
    const char *chan_name;

    input = gtk_entry_get_text(chan->input_entry);
    chan_name = gtk_widget_get_name(GTK_WIDGET(chan));

    LOG_FR("panel = %s, text = '%s'", chan_name, input);

    if (input[0] == '/'){
        res = srain_cmd(chan_name, input);
    } else {
        res = srain_send(chan_name, input);
    }
    if (res != -1)
        gtk_entry_set_text(chan->input_entry, "");
}

static int msg_list_box_popup(GtkWidget *widget, GdkEventButton *event, gpointer *user_data){
    GtkMenu *menu;

    menu = GTK_MENU(user_data);
    if (event->button == 3){
        gtk_menu_popup(menu, NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

static void srain_chan_sys_msg_addf(SrainChan *chan, sys_msg_type_t type, const char *fmt, ...){
    char msg[512];
    va_list args;

    if (strlen(fmt) != 0 ){
        // Format the data
        va_start(args, fmt);
        vsnprintf(msg, sizeof (msg), fmt, args);
        va_end(args);

        srain_chan_sys_msg_add(chan, type, msg);
    }
}

static void srain_chan_init(SrainChan *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    self->last_msg = NULL;
    theme_apply(GTK_WIDGET(self->msg_menu));

    g_signal_connect_swapped(self->input_entry, "activate", G_CALLBACK(on_send), self);
    g_signal_connect_swapped(self->send_button, "clicked", G_CALLBACK(on_send), self);
    g_signal_connect(self->onlinelist_button, "clicked", G_CALLBACK(onlinelist_button_on_click), self->revealer);
    g_signal_connect(self->online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);
    g_signal_connect(self->msg_box, "button_press_event", G_CALLBACK(msg_list_box_popup), self->msg_menu);
}

static void srain_chan_class_init(SrainChanClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chan.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_scrolledwindow);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, onlinelist_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, online_listbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, send_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, input_entry);
}

SrainChan* srain_chan_new(const char *name){
    SrainChan *chan;

    chan = g_object_new(SRAIN_TYPE_CHAN, NULL);
    gtk_label_set_text(chan->name_label, name);
    gtk_widget_set_name(GTK_WIDGET(chan), name);

    return chan;
}

void srain_chan_set_topic(SrainChan *chan, const char *topic){
    gtk_label_set_text(chan->topic_label, topic);
}

/**
 * @brief srain_chan_online_list_add 
 *
 * @param chan
 * @param name
 * @param is_init if is_init = 1, sys msg will not be sent
 */
void srain_chan_online_list_add(SrainChan *chan, const char *name, int is_init){
    const char *chan_name;
    GtkWidget *label;
    GtkListBoxRow *row;

    chan_name =gtk_widget_get_name(GTK_WIDGET(chan));
    row = get_list_item_by_name(chan->online_listbox, name);
    if (row){
        ERR_FR("GtkListBoxRow %s already exist in %s", name, chan_name);
        return;
    }
    label = gtk_label_new(name);
    gtk_widget_set_name(label, name);

    gtk_container_add(GTK_CONTAINER(chan->online_listbox), label);
    theme_apply(GTK_WIDGET(chan->online_listbox));
    gtk_widget_show(label);

    if (!is_init)
        srain_chan_sys_msg_addf(chan, SYS_MSG_NORMAL, "%s has joined %s", name, chan_name);
}

void srain_chan_online_list_rm(SrainChan *chan, const char *name, const char *reason){
    const char *chan_name;
    GtkListBoxRow *row;

    chan_name =gtk_widget_get_name(GTK_WIDGET(chan));
    row = get_list_item_by_name(chan->online_listbox, name);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found in %s", name, chan_name);
        return;
    }
    gtk_container_remove(GTK_CONTAINER(chan->online_listbox), GTK_WIDGET(row));

    srain_chan_sys_msg_addf(chan, SYS_MSG_NORMAL, "%s has left %s: %s", name, chan_name, reason);
}

void srain_chan_online_list_rename(SrainChan *chan, const char *old_name, const char *new_name){
    const char *chan_name;
    GtkLabel *label;
    GtkListBoxRow *row;

    chan_name =gtk_widget_get_name(GTK_WIDGET(chan));
    row = get_list_item_by_name(chan->online_listbox, old_name);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found in %s", old_name, chan_name);
        return;
    }

    label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row)));
    gtk_label_set_text(label, new_name);

    srain_chan_sys_msg_addf(chan, SYS_MSG_NORMAL, "%s is now known as %s", old_name, new_name);
}

void srain_chan_sys_msg_add(SrainChan *chan, sys_msg_type_t type, const char *msg){
    SrainSysMsg *smsg;

    smsg = srain_sys_msg_new(type, msg);

    gtk_container_add(GTK_CONTAINER(chan->msg_box), GTK_WIDGET(smsg));
    theme_apply(GTK_WIDGET(chan->msg_box));
    gtk_widget_show(GTK_WIDGET(smsg));

    chan->last_msg = GTK_WIDGET(smsg);

    scroll_to_bottom(chan);
}


void srain_chan_send_msg_add(SrainChan *chan, const char *msg, const char *img_path){
    SrainSendMsg *smsg;

    smsg = srain_send_msg_new(msg, img_path);
    gtk_container_add(GTK_CONTAINER(chan->msg_box), GTK_WIDGET(smsg));
    theme_apply(GTK_WIDGET(chan->msg_box));
    gtk_widget_show(GTK_WIDGET(smsg));

    chan->last_msg = GTK_WIDGET(smsg);

    scroll_to_bottom(chan);
}

void _srain_chan_recv_msg_add(SrainChan *chan, const char *nick, const char *id, const char *msg, const char *img_path){
    SrainRecvMsg *smsg;

    smsg = srain_recv_msg_new(nick, id, msg, img_path);
    gtk_container_add(GTK_CONTAINER(chan->msg_box), GTK_WIDGET(smsg));
    theme_apply(GTK_WIDGET(chan->msg_box));
    gtk_widget_show(GTK_WIDGET(smsg));

    chan->last_msg = GTK_WIDGET(smsg);
}

/* add a SrainRecvMsg into SrainChan, if its time is same to the last msg, combine them */
void srain_chan_recv_msg_add(SrainChan *chan, const char *nick, const char *id, const char *msg, const char *img_path){
    char timestr[32];
    const char *old_timestr;
    const char *old_nick;
    const char *old_msg;
    GString *new_msg;
    SrainRecvMsg *last_recv_msg;

    get_cur_time(timestr);
    if (chan->last_msg && SRAIN_IS_RECV_MSG(chan->last_msg)){
        last_recv_msg = SRAIN_RECV_MSG(chan->last_msg);
        old_timestr = gtk_label_get_text(last_recv_msg->time_label);
        old_nick = gtk_label_get_text(last_recv_msg->nick_label);

        if (strncmp(timestr, old_timestr, 32) == 0
                && strncmp(nick, old_nick, NICK_LEN) == 0){
            old_msg = gtk_label_get_text(last_recv_msg->msg_label);
            new_msg = g_string_new(old_msg);
            g_string_append(new_msg, "\n\n");
            g_string_append(new_msg, msg);

            gtk_label_set_text(last_recv_msg->msg_label, new_msg->str);

            g_string_free(new_msg, TRUE);

            scroll_to_bottom(chan);
            return;
        }
    }

    _srain_chan_recv_msg_add(chan, nick, id, msg, img_path);
    scroll_to_bottom(chan);
}
