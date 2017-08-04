/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file srain_msg.c
 * @brief GtkWidget subclass used to display message
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06
 * @date 2016-03-01
 */

#include <time.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "nick_menu.h"
#include "srain_window.h"
#include "srain_chat.h"
#include "srain_msg.h"

#include "plugin.h"
#include "download.h"
#include "log.h"
#include "file_helper.h"
#include "i18n.h"
#include "meta.h"
#include "utils.h"

/**
 * @brief Get the selected text (utf-8 supported) of `label`,
 *      if no text was selected, return all of the text in this label,
 *      if there is any '\n'(newline) in the text, strip it.
 *
 * @return A allocated (char *), it should be freed by `free()`
 */
static char* label_get_selection(GtkLabel *label){
    int start, end;
    const char *msg;
    char *sel_msg;
    if (!label) return NULL;

    msg = gtk_label_get_text(label);

    if (gtk_label_get_selection_bounds(label, &start, &end)){
        sel_msg = g_utf8_substring(msg, start, end);
    } else {
        sel_msg = strdup(msg);
    }

    return sel_msg;
}

static void copy_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char* sel_msg;
    char* line;
    GString *str;
    GtkClipboard *cb;
    SrainRecvMsg *smsg;

    smsg = SRAIN_RECV_MSG(user_data);

    if ((sel_msg = label_get_selection(smsg->msg_label)) == NULL) return;

    /* Get the clipboard object */
    cb = gtk_widget_get_clipboard(GTK_WIDGET(smsg), GDK_SELECTION_CLIPBOARD);

    str = g_string_new("");
    line = strtok(sel_msg, "\n");
    while (line){
        g_string_append_printf(str, "[%s] <%s> %s\n",
                gtk_label_get_text(smsg->time_label),
                gtk_label_get_text(smsg->nick_label),
                line);
        line = strtok(NULL, "\n");
    }
    /* Set clipboard text */
    gtk_clipboard_set_text(cb, str->str, -1);

    g_string_free(str, TRUE);
    g_free(sel_msg);
}

static void froward_submenu_item_on_activate(GtkWidget* widget, gpointer user_data){
    int count;
    char *sel_msg;
    char *line;
    const char *remark;
    GString *str;
    SrainChat *chat;
    SrainRecvMsg *smsg;
    const char *params[1];

    smsg = SRAIN_RECV_MSG(user_data);
    if ((sel_msg = label_get_selection(smsg->msg_label)) == NULL) return;

    remark = srain_chat_get_remark(srain_window_get_cur_chat(srain_win));
    chat = srain_window_get_chat(srain_win,
            gtk_menu_item_get_label(GTK_MENU_ITEM(widget)),
            remark);

    line = strtok(sel_msg, "\n");
    while (line){
        str = g_string_new("");
        g_string_printf(str, _("%s <fwd %s@%s>"), line,
                gtk_label_get_text(smsg->nick_label),
                srain_chat_get_name(srain_window_get_cur_chat(srain_win)));
        line = strtok(NULL, "\n");

        count = 0;
        params[count++] = str->str;
        sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_SEND, params, count);

        g_string_free(str, TRUE);
    }

    g_free(sel_msg);
}

static void msg_label_on_popup(GtkLabel *label, GtkMenu *menu,
        gpointer user_data){
    GtkMenuItem *copy_menu_item;
    GtkMenuItem *forward_menu_item;
    SrainRecvMsg *smsg;

    smsg = SRAIN_RECV_MSG(user_data);

    /* Create menuitem copy_menu_item */
    copy_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Copy message")));
    gtk_widget_show(GTK_WIDGET(copy_menu_item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(copy_menu_item));
    g_signal_connect(copy_menu_item, "activate",
                G_CALLBACK(copy_menu_item_on_activate), smsg);

    /* Create menuitem forward_menu_item */
    forward_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Forward to...")));
    gtk_widget_show(GTK_WIDGET(forward_menu_item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(forward_menu_item));

    /* Create submenu of forward_menu_item */
    GList *chats;
    GtkMenuItem *item;
    GtkMenu *forward_submenu = GTK_MENU(gtk_menu_new());
    SrainChat *chat;

    chat = srain_window_get_cur_chat(srain_win);

    chats = srain_window_get_chats_by_remark(srain_win,
            srain_chat_get_remark(chat));
    if (!chats) return;
    /* Skip META_SERVER */
    chats = g_list_next(chats);

    gtk_menu_item_set_submenu(forward_menu_item, GTK_WIDGET(forward_submenu));

    while (chats){
        item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                    srain_chat_get_name(chats->data)));
        gtk_widget_show(GTK_WIDGET(item));
        g_signal_connect(item, "activate",
                G_CALLBACK(froward_submenu_item_on_activate), smsg);
        gtk_menu_shell_append(GTK_MENU_SHELL(forward_submenu), GTK_WIDGET(item));

        chats = g_list_next(chats);
    }

    g_list_free(chats);
}

static gboolean nick_button_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    GtkLabel *nick_label;

    nick_label = GTK_LABEL(user_data);
    if (event->button == 3){
        nick_menu_popup(event, gtk_label_get_text(nick_label));

        return TRUE;
    }
    return FALSE;
}

static void nick_button_on_click(GtkWidget *widget, gpointer *user_data){
    GString *str;
    GtkLabel *nick_label;

    nick_label = GTK_LABEL(user_data);
    str = g_string_new(gtk_label_get_text(nick_label));
    str = g_string_append(str, ": ");

    srain_chat_insert_text(srain_window_get_cur_chat(srain_win), str->str, 0);
    g_string_free(str, TRUE);
}

/* ================ SRAIN_SRAIN_MSG ================ */
void srain_msg_append_image(SrainMsg *smsg, const char *url, SrainImageFlag flag) {
    SrainImage *simg = srain_image_new();
    srain_image_set_from_url_async(simg, url, 300, flag);

    gtk_container_add(GTK_CONTAINER(smsg->padding_box), GTK_WIDGET(simg));
    gtk_container_set_border_width(GTK_CONTAINER(simg), 6);
    gtk_widget_show(GTK_WIDGET(simg));
}

void srain_msg_set_msg(SrainMsg *smsg, const char *msg) {
    gtk_label_set_markup(smsg->msg_label, msg);
}

int srain_msg_append_msg(SrainMsg *smsg, const char *msg) {
    const char *old_markup;
    GString *new_markup;

    old_markup = gtk_label_get_label(smsg->msg_label);
    new_markup = g_string_new(msg);

    g_string_prepend(new_markup, "\n");
    g_string_prepend(new_markup, old_markup);

    gtk_label_set_markup(smsg->msg_label, new_markup->str);

    g_string_free(new_markup, TRUE);

    return 0;
}

void srain_msg_set_time(SrainMsg *smsg, time_t time) {
    char timestr[32];
    char tooltip[64];

    time_to_str(time, timestr, sizeof(timestr), "%R");
    time_to_str(time, tooltip, sizeof(tooltip), _("%Y-%m-%d %T"));

    gtk_label_set_text(smsg->time_label, timestr);
    gtk_widget_set_tooltip_text(GTK_WIDGET(smsg->time_label), tooltip);
}


/* ================ SRAIN_SYS_MSG ================ */
G_DEFINE_TYPE(SrainSysMsg, srain_sys_msg, GTK_TYPE_BOX);

static void srain_sys_msg_init(SrainSysMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_sys_msg_class_init(SrainSysMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/sys_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSysMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSysMsg, time_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSysMsg, padding_box);
}

SrainSysMsg* srain_sys_msg_new(const char *msg, SysMsgType type){
    SrainSysMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SYS_MSG, NULL);

    switch (type){
        case SYS_MSG_NORMAL:
            gtk_widget_set_name(GTK_WIDGET(smsg), "normal_sys_msg_box");
            break;
        case SYS_MSG_ERROR:
            gtk_widget_set_name(GTK_WIDGET(smsg), "error_sys_msg_box");
            break;
        case SYS_MSG_ACTION:
            gtk_widget_set_name(GTK_WIDGET(smsg), "action_sys_msg_box");
            break;
        default:
            ERR_FR("unkown SysMsgType");
    }

    smsg->type = type;
    srain_msg_set_msg(SRAIN_MSG(smsg), msg);

    return smsg;
}

/* ================ SRAIN_SEND_MSG ================ */
G_DEFINE_TYPE(SrainSendMsg, srain_send_msg, GTK_TYPE_BOX);

static void srain_send_msg_init(SrainSendMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_send_msg_class_init(SrainSendMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/send_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, padding_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, time_label);
}

SrainSendMsg* srain_send_msg_new(const char *msg){
    SrainSendMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SEND_MSG, NULL);
    srain_msg_set_msg(SRAIN_MSG(smsg), msg);

    return smsg;
}

/* ================ SRAIN_RECV_MSG ================ */
G_DEFINE_TYPE(SrainRecvMsg, srain_recv_msg, GTK_TYPE_BOX);

static void srain_recv_msg_init(SrainRecvMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(self->nick_button, "clicked",
            G_CALLBACK(nick_button_on_click), self->nick_label);
    g_signal_connect(self->nick_button, "button-press-event",
            G_CALLBACK(nick_button_on_popup), self->nick_label);
    g_signal_connect(self->msg_label, "populate-popup",
            G_CALLBACK(msg_label_on_popup), self);

}

static void srain_recv_msg_class_init(SrainRecvMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/recv_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, avatar_image);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, padding_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, time_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, nick_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, identify_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, nick_button);
}

static gboolean set_avatar_retry_timeout(gpointer user_data){
    long left;
    char *avatar_path;
    GdkPixbuf *pixbuf;
    SrainRecvMsg *smsg;

    /* Check whether object is alive now */
    g_return_val_if_fail(SRAIN_IS_RECV_MSG(user_data), FALSE);

    smsg = SRAIN_RECV_MSG(user_data);

    left = (long) g_object_get_data(G_OBJECT(smsg), "left-times");
    if (left == 0) return TRUE;

    avatar_path = get_avatar_file(gtk_label_get_text(smsg->nick_label));

    if (avatar_path){
        gtk_widget_show(GTK_WIDGET(smsg->avatar_image));
        pixbuf = gdk_pixbuf_new_from_file_at_size(
                avatar_path, 36, 36, NULL);
        if (pixbuf){
            gtk_image_set_from_pixbuf(smsg->avatar_image, pixbuf);
            g_object_unref(pixbuf);
        }
        g_free(avatar_path);
    } else {
        g_object_set_data(G_OBJECT(smsg), "left-times", (void *)(left - 1));
        return TRUE;
    }

    return FALSE;
}

SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg){
    SrainRecvMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_RECV_MSG, NULL);
    gtk_label_set_text(smsg->nick_label, nick);
    gtk_label_set_text(smsg->identify_label, id);
    srain_msg_set_msg(SRAIN_MSG(smsg), msg);

    return smsg;
}

void srain_recv_msg_show_avatar(SrainRecvMsg *smsg, bool isshow){
    if (isshow){
        const char *nick;
        char *avatar_path;
        GdkPixbuf *pixbuf;

        nick = gtk_label_get_text(smsg->nick_label);
        avatar_path = get_avatar_file(nick);

        if (avatar_path){
            // gtk_widget_show(GTK_WIDGET(smsg->avatar_image));
            pixbuf = gdk_pixbuf_new_from_file_at_size(
                    avatar_path, 36, 36, NULL);
            if (pixbuf){
                gtk_image_set_from_pixbuf(smsg->avatar_image, pixbuf);
                g_object_unref(pixbuf);
            }
            g_free(avatar_path);
        } else {
            g_object_set_data(G_OBJECT(smsg), "left-times", (void *)5);
            g_timeout_add(5000, (GSourceFunc)set_avatar_retry_timeout, smsg);
        }
    }

    gtk_widget_set_visible(GTK_WIDGET(smsg->avatar_image), isshow);
}
