/**
 * @file srain_chat.c
 * @brief Complex widget used to reprsenting a session
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "theme.h"
#include "ui.h"
#include "ui_common.h"
#include "ui_hdr.h"
#include "srain_chat.h"
#include "srain_entry_completion.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_msg.h"

#include "markup.h"
#include "plugin.h"
#include "log.h"
#include "i18n.h"

#include "cmd_list.h"

struct _SrainChat {
    GtkBox parent;

    char *server_name;
    char *chat_name;
    ChatType type;

    /* header */
    GtkLabel* name_label;
    GtkMenu *menu;
    GtkRevealer *topic_revealer;
    GtkLabel *topic_label;

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

    g_return_if_fail(SRAIN_IS_CHAT(user_data));
    chat = user_data;

    ui_hdr_srv_part(chat, "");
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

    if (url) {
        g_object_set_data(G_OBJECT(entry), "image-url", url);
        gdk_threads_add_idle((GSourceFunc)upload_image_idle, entry);
    }
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
    char *input;
    const char *chat_name;

    input = strdup(gtk_entry_get_text(chat->input_entry));
    chat_name = gtk_widget_get_name(GTK_WIDGET(chat));

    if (is_blank(input)) goto ret;

    LOG_FR("chat: %s, text: '%s'", chat_name, input);

    if (input[0] == '/'){
        ui_hdr_srv_cmd(chat, input, 0);
    } else {
        ui_send_msg_sync(
                    srain_chat_get_srv_name(chat),
                    srain_chat_get_chat_name(chat),
                    input);
        if (ui_hdr_srv_send(chat, input) < 0){
            ui_sys_msg_sync(
                    srain_chat_get_srv_name(chat),
                    srain_chat_get_chat_name(chat),
                    _("Failed to send message"),
                    SYS_MSG_ERROR);
        }
    }

ret:
    gtk_entry_set_text(chat->input_entry, "");
    free(input);
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

    /* command completion */
    int i;
    for (i = 0; cmd_list[i] != 0; i++){
        srain_entry_completion_add_keyword(self->completion,
                cmd_list[i], KEYWORD_NORMAL);
    }

}

static void srain_chat_finalize(GObject *object){
    free(SRAIN_CHAT(object)->server_name);
    free(SRAIN_CHAT(object)->chat_name);

    G_OBJECT_CLASS(srain_chat_parent_class)->finalize(object);
}

static void srain_chat_class_init(SrainChatClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    object_class->finalize = srain_chat_finalize;

    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chat.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, topic_revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, topic_label);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, toggle_topic_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, close_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, toggle_user_list_menu_item);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, msg_list_box);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, user_list_revealer);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, nick_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, input_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChat, upload_image_button);
}

SrainChat* srain_chat_new(const char *server_name, const char *chat_name,
        ChatType type){
    SrainChat *chat;

    chat = g_object_new(SRAIN_TYPE_CHAT, NULL);

    chat->type =type;

    gtk_label_set_text(chat->name_label, chat_name);
    gtk_widget_set_name(GTK_WIDGET(chat), chat_name);

    chat->chat_name = strdup(chat_name);
    chat->server_name = strdup(server_name);

    switch (chat->type){
        case CHAT_SERVER:
            gtk_menu_item_set_label(chat->close_menu_item, _("Disconnect"));
            break;
        case CHAT_CHANNEL:
            gtk_menu_item_set_label(chat->close_menu_item, _("Leave"));
            break;
        case CHAT_PRIVATE:
            gtk_menu_item_set_label(chat->close_menu_item, _("Close"));
            break;
        default:
            break;
    }

    return chat;
}

void srain_chat_set_topic(SrainChat *chat, const char *topic){
    GString *markuped_topic;

    markuped_topic = markup(topic, NULL);
    if (markuped_topic){
        gtk_label_set_markup(chat->topic_label, markuped_topic->str);
        g_string_free(markuped_topic, TRUE);
    } else {
        gtk_label_set_text(chat->topic_label, topic);
    }
}

/**
 * @brief Insert text into a SrainChat's input entry
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

const char* srain_chat_get_name(SrainChat *chat){
    return chat->chat_name;
}

const char* srain_chat_get_srv_name(SrainChat *chat){
    return chat->server_name;
}

const char* srain_chat_get_chat_name(SrainChat *chat){
    return chat->chat_name;
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
