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
 * @file srain_chat.c
 * @brief Srain's chat panel
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */


#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "sui/sui.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "srain_chat.h"
#include "srain_entry_completion.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_msg.h"
#include "theme.h"

#include "plugin.h"
#include "log.h"
#include "i18n.h"

struct _SrainChat {
    GtkBox parent;

    ChatType type;
    SuiSession *session;

    /* header */
    GtkLabel* name_label;
    GtkLabel* remark_label;
    GtkMenu *menu;
    GtkRevealer *topic_revealer;
    GtkLabel *topic_label;
    GtkLabel *topic_setter_label;

    /* Menu */
    GtkMenuItem *toggle_topic_menu_item;
    GtkMenuItem *toggle_user_list_menu_item;
    GtkMenuItem *close_menu_item;

    GtkBox *msg_list_box;    // SrainMsgList container
    SrainMsgList *msg_list;

    GtkRevealer *user_list_revealer;    // SrainUserList container
    SrainUserList *user_list;

    /* input entry */
    GtkLabel *nick_label;
    GtkEntry *input_entry;
    SrainEntryCompletion *completion;
    GtkButton *upload_image_button;

    GtkWidget *last_msg;
};

struct _SrainChatClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChat, srain_chat, GTK_TYPE_BOX);

static void toggle_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    GtkRevealer *revealer;

    revealer = user_data;

    gtk_revealer_set_reveal_child(revealer,
            !gtk_revealer_get_reveal_child(revealer));
}

static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SrainChat *chat;

    chat = user_data;

    switch (chat->type){
        case CHAT_SERVER:
            sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_DISCONNECT, NULL, 0);
            break;
        case CHAT_CHANNEL:
            sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_PART, NULL, 0);
            break;
        case CHAT_PRIVATE:
            sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_UNQUERY, NULL, 0);
            break;
        default:
            break;
    }
}

static gboolean entry_on_key_press(gpointer user_data, GdkEventKey *event){
    SrainChat *chat;

    chat = user_data;
    switch (event->keyval){
        case GDK_KEY_Down:
            // TODO: use up/down to switch history message?
            srain_msg_list_scroll_down(chat->msg_list, 30);
            break;
        case GDK_KEY_Up:
            srain_msg_list_scroll_up(chat->msg_list, 30);
            break;
        case GDK_KEY_Tab:
            srain_entry_completion_complete(chat->completion);
            break;
        case GDK_KEY_n:
            if (event->state & GDK_CONTROL_MASK){
                srain_entry_completion_complete(chat->completion);
                break;
            }
        default:
            return FALSE;
    }

    return TRUE;
}

static gboolean upload_image_idle(GtkEntry *entry){
    char *url;

    /* Check whether object is alive now */
    g_return_val_if_fail(GTK_IS_ENTRY(entry), FALSE);

    url = g_object_get_data(G_OBJECT(entry), "image-url");
    if (url){
        gtk_entry_set_text(entry, url);
        g_free(url);
    } else {
        gtk_entry_set_text(entry, _("Failed to upload image"));
    }

    g_object_set_data(G_OBJECT(entry), "image-url", NULL);
    gtk_widget_set_sensitive(GTK_WIDGET(entry), TRUE);

    /* NOTE: DON'T FORGET to return FALSE!!! */
    return FALSE;
}

static void upload_image_async(GtkEntry *entry){
    char *url;
    const char *filename;

    filename = gtk_entry_get_text(entry);
    url = plugin_upload(filename);

    g_object_set_data(G_OBJECT(entry), "image-url", url);
    gdk_threads_add_idle((GSourceFunc)upload_image_idle, entry);
}

static void upload_image_button_on_click(GtkWidget *widget, gpointer user_data){
    char *filename;
    GtkEntry *entry;
    GtkWindow *toplevel;

    entry = user_data;

    toplevel = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    filename = show_open_filechosser(toplevel);
    if (filename) {
        LOG_FR("filename: '%s'", filename);
        gtk_widget_set_sensitive(GTK_WIDGET(entry), FALSE);
        gtk_entry_set_text(entry, filename);

        g_thread_new(NULL, (GThreadFunc)upload_image_async, entry);

        g_free(filename);
    }

}

static int is_blank(const char *str){
    while (*str){
        if (*str != '\t' && *str != ' ')
            return 0;
        str++;
    }
    return 1;
}

static void input_entry_on_activate(SrainChat *chat){
    int count;
    char *input;
    const char *params[1];

    input = g_strdup(gtk_entry_get_text(chat->input_entry));

    if (is_blank(input)) goto ret;

    count = 0;
    params[count++] = input;
    sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_SEND, params, count);

ret:
    gtk_entry_set_text(chat->input_entry, "");
    g_free(input);

    return;
}

static void srain_chat_init(SrainChat *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    /* init completion list */
    self->completion = srain_entry_completion_new(self->input_entry);

    /* init user list */
    self->user_list = srain_user_list_new();
    gtk_container_add(GTK_CONTAINER(self->user_list_revealer),
            GTK_WIDGET(self->user_list));
    gtk_widget_show(GTK_WIDGET(self->user_list));

    /* init msg list */
    self->msg_list = srain_msg_list_new();
    gtk_box_pack_start(self->msg_list_box, GTK_WIDGET(self->msg_list),
            TRUE, TRUE, 0);
    gtk_widget_show(GTK_WIDGET(self->msg_list));

    self->last_msg = NULL;

    /* Menu */
    g_signal_connect(self->toggle_topic_menu_item, "activate",
            G_CALLBACK(toggle_menu_item_on_activate), self->topic_revealer);
    g_signal_connect(self->toggle_user_list_menu_item, "activate",
            G_CALLBACK(toggle_menu_item_on_activate), self->user_list_revealer);
    g_signal_connect(self->close_menu_item, "activate",
            G_CALLBACK(close_menu_item_on_activate), self);

    g_signal_connect_swapped(self->input_entry, "activate",
            G_CALLBACK(input_entry_on_activate), self);
    g_signal_connect_swapped(self->input_entry, "key_press_event",
            G_CALLBACK(entry_on_key_press), self);

    g_signal_connect(self->upload_image_button, "clicked",
            G_CALLBACK(upload_image_button_on_click), self->input_entry);
}

static void srain_chat_finalize(GObject *object){
    G_OBJECT_CLASS(srain_chat_parent_class)->finalize(object);
}

static void srain_chat_class_init(SrainChatClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    object_class->finalize = srain_chat_finalize;

    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chat.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, remark_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, topic_revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, topic_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, topic_setter_label);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, toggle_topic_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, close_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, toggle_user_list_menu_item);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, msg_list_box);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, user_list_revealer);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, nick_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, input_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, upload_image_button);
}

SrainChat* srain_chat_new(SuiSession *sui, const char *name, const char *remark,
        ChatType type){
    SrainChat *chat;

    chat = g_object_new(SRAIN_TYPE_CHAT, NULL);

    chat->type =type;
    chat->session = sui;

    gtk_label_set_text(chat->name_label, name);
    gtk_label_set_text(chat->remark_label, remark);
    gtk_widget_set_name(GTK_WIDGET(chat), name);

    switch (chat->type){
        case CHAT_SERVER:
            gtk_menu_item_set_label(chat->close_menu_item, _("Disconnect (_C)"));
            break;
        case CHAT_CHANNEL:
            gtk_menu_item_set_label(chat->close_menu_item, _("Leave (_C)"));
            break;
        case CHAT_PRIVATE:
            gtk_menu_item_set_label(chat->close_menu_item, _("_Close"));
            break;
        default:
            break;
    }

    return chat;
}

void srain_chat_set_topic(SrainChat *chat, const char *topic){
    gtk_label_set_markup(chat->topic_label, topic);
    gtk_widget_show(GTK_WIDGET(chat->topic_label));
}

void srain_chat_set_topic_setter(SrainChat *chat, const char *setter){
    gtk_label_set_text(chat->topic_setter_label, setter);
    gtk_widget_show(GTK_WIDGET(chat->topic_setter_label));
}

/**
 * @brief Insert text into a SrainChat's 0nput entry
 *
 * @param chat
 * @param text
 * @param pos If the pos = -1, insert at current position
 */
void srain_chat_insert_text(SrainChat *chat, const char *text, int pos){
    GtkEntryBuffer *buf;

    buf = gtk_entry_get_buffer(chat->input_entry);
    if (pos == -1)
        pos = gtk_editable_get_position(GTK_EDITABLE(chat->input_entry));

    gtk_entry_buffer_insert_text(buf, pos, text, -1);
    gtk_editable_set_position(GTK_EDITABLE(chat->input_entry),
            pos + strlen(text));
}

void srain_chat_fcous_entry(SrainChat *chat){
    gtk_widget_grab_focus(GTK_WIDGET(chat->input_entry));
}

SrainUserList* srain_chat_get_user_list(SrainChat *chat){
    return chat->user_list;
}

SrainMsgList* srain_chat_get_msg_list(SrainChat *chat){
    return chat->msg_list;
}

SrainEntryCompletion* srain_chat_get_entry_completion(SrainChat *chat){
    return chat->completion;
}

void srain_chat_set_name(SrainChat *chat, const char *name){
    gtk_label_set_text(chat->name_label, name);
}

const char* srain_chat_get_name(SrainChat *chat){
    return gtk_label_get_text(chat->name_label);
}

void srain_chat_set_remark(SrainChat *chat, const char *remark){
    gtk_label_set_text(chat->remark_label, remark);
}

const char* srain_chat_get_remark(SrainChat *chat){
    return gtk_label_get_text(chat->remark_label);
}

void srain_chat_set_nick(SrainChat *chat, const char *nick){
    gtk_label_set_text(chat->nick_label, nick);
}

const char* srain_chat_get_nick(SrainChat *chat){
    return gtk_label_get_text(chat->nick_label);
}

ChatType srain_chat_get_chat_type(SrainChat *chat){
    return chat->type;
}

GtkMenu* srain_chat_get_menu(SrainChat *chat){
    return chat->menu;
}

SuiSession *srain_chat_get_session(SrainChat *chat){
    return chat->session;
}

void srain_chat_show_topic(SrainChat *chat, bool isshow){
    gtk_revealer_set_reveal_child(chat->topic_revealer, isshow);
}

void srain_chat_show_user_list(SrainChat *chat, bool isshow){
    gtk_revealer_set_reveal_child(chat->user_list_revealer, isshow);
}
