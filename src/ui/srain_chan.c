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

#include "theme.h"
#include "ui_common.h"
#include "ui_intf.h"
#include "srain_chan.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_msg.h"

#include "markup.h"
#include "plugin.h"
#include "log.h"

#include "cmd_list.h"

struct _SrainChan {
    GtkBox parent;

    /* header */
    GtkLabel* name_label;
    GtkRevealer *topic_revealer;
    GtkLabel *topic_label;

    /* option box */
    GtkBox *option_box;
    GtkToggleButton *show_topic_togglebutton;
    GtkToggleButton *show_user_list_togglebutton;
    GtkButton *leave_button;

    GtkBox *msg_list_box;    // SrainMsgList container
    SrainMsgList *msg_list;

    GtkRevealer *user_list_revealer;
    GtkViewport *user_list_viewport;    // SrainUserList container
    SrainUserList *user_list;

    /* input entry */
    GtkEntry *input_entry;
    GtkEntryCompletion *entrycompletion;
    GtkListStore *completion_list;
    GtkButton *upload_image_button;

    GtkWidget *last_msg;
};

struct _SrainChanClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChan, srain_chan, GTK_TYPE_BOX);

static void entry_auto_completion(GtkEntry *entry){
    int cur_pos;
    const char *word_ptr;
    const char *text;
    const char *word;
    const char *prefix;
    GtkEntryBuffer *buf;
    GtkEntryCompletion *comp;

    comp = gtk_entry_get_completion(entry);
    buf = gtk_entry_get_buffer(entry);
    text = gtk_entry_get_text(entry);

    cur_pos = gtk_editable_get_position(GTK_EDITABLE(entry));
    LOG_FR("current position %d", cur_pos);
    word_ptr = text + cur_pos;

    while (word_ptr > text){
        word_ptr = g_utf8_prev_char(word_ptr);
        if (*word_ptr == ' '){
            word_ptr++;
            break;
        }
    }
    word = strndup(word_ptr, text + cur_pos - word_ptr);
    LOG_FR("word '%s'", word);
    // TODO: 中文处理有问题

    prefix = gtk_entry_completion_compute_prefix(comp, word);
    LOG_FR("prefix '%s'", prefix);
    if (prefix) {
        gtk_entry_buffer_insert_text(buf, cur_pos, prefix + strlen(word), -1);
        gtk_editable_set_position(GTK_EDITABLE(entry),
                cur_pos + strlen(prefix) - strlen(word));
        // gtk_editable_select_region(GTK_EDITABLE(entry),
                // cur_pos, cur_pos + strlen(prefix) - strlen(word));
    }
}

static gboolean entry_on_key_press(gpointer user_data, GdkEventKey *event){
    SrainChan *chan;

    chan = user_data;
    switch (event->keyval){
        case GDK_KEY_Down:
            // srain_chan_scroll_down(chan);
            break;
        case GDK_KEY_Up:
            // srain_chan_scroll_up(chan);
            break;
        case GDK_KEY_Tab:
            entry_auto_completion(chan->input_entry);
            break;
        case GDK_KEY_n:
            if (event->state & GDK_CONTROL_MASK){
                entry_auto_completion(chan->input_entry);
                break;
            }
        default:
            return FALSE;
    }

    return TRUE;
}

static void upload_image_idle(GtkEntry *entry){
    char *url;

    url = g_object_get_data(G_OBJECT(entry), "image-url");
    if (url){
        gtk_entry_set_text(entry, url);
        free(url);
    } else {
        gtk_entry_set_text(entry, "upload image failed");
    }

    gtk_widget_set_sensitive(GTK_WIDGET(entry), TRUE);
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

static void leave_button_on_click(GtkWidget *widget, gpointer user_data){
    SrainChan *chan;

    chan = user_data;
    // TODO: unquery
    ui_intf_server_part(chan);
}

static void option_togglebutton_on_click(GtkWidget *widget, gpointer user_data){
    GtkRevealer *revealer;
    GtkToggleButton *button;

    revealer = user_data;
    button = GTK_TOGGLE_BUTTON(widget);
    gtk_revealer_set_reveal_child(revealer,
            gtk_toggle_button_get_active(button));
}

static gint online_listbox_on_dbclick(GtkWidget *widget, GdkEventButton *event){
    GString *cmd;
    GtkLabel *label;
    GtkListBoxRow *row;

    if(event->button == 1 && event->type == GDK_2BUTTON_PRESS){
        row = gtk_list_box_get_selected_row(GTK_LIST_BOX(widget));
        if (row){
            label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row)));
            cmd = g_string_new(NULL);

            // server = g_object_get_data(G_OBJECT(chan), "server");
            g_string_printf(cmd, "/whois %s", gtk_label_get_text(label));
            // irc_server_cmd(NULL, NULL, cmd->str);
            g_string_free(cmd, TRUE);
        }
    }
    return FALSE;
}

static int is_blank(const char *str){
    while (*str){
        if (*str != '\t' && *str != ' ')
            return 0;
        str++;
    }
    return 1;
}

static void input_entry_on_activate(SrainChan *chan){
    char *input;
    const char *chan_name;

    input = strdup(gtk_entry_get_text(chan->input_entry));
    chan_name = gtk_widget_get_name(GTK_WIDGET(chan));

    if (is_blank(input)) goto ret;

    LOG_FR("chan: %s, text: '%s'", chan_name, input);

    if (input[0] == '/'){
        ui_intf_server_cmd(chan, input);
    } else {
        ui_intf_server_send(chan, input);
    }

ret:
    gtk_entry_set_text(chan->input_entry, "");
    free(input);
    return;
}

// TODO: NOT work now
static gboolean msg_list_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer *user_data){
    GtkMenu *menu;

    menu = GTK_MENU(user_data);
    if (event->button == 3){
        gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

static void srain_chan_init(SrainChan *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    /* init completion list */
    self->completion_list = gtk_list_store_new(1, G_TYPE_STRING);

    /* init user list */
    self->user_list = srain_user_list_new();
    gtk_container_add(GTK_CONTAINER(self->user_list_viewport),
            GTK_WIDGET(self->user_list));
    gtk_widget_show(GTK_WIDGET(self->user_list));

    /* init msg list */
    self->msg_list = srain_msg_list_new();
    gtk_box_pack_start(self->msg_list_box, GTK_WIDGET(self->msg_list),
            TRUE, TRUE, 0);
    gtk_widget_show(GTK_WIDGET(self->msg_list));

    self->last_msg = NULL;

    g_signal_connect_swapped(self->input_entry, "activate",
            G_CALLBACK(input_entry_on_activate), self);
    g_signal_connect_swapped(self->input_entry, "key_press_event",
            G_CALLBACK(entry_on_key_press), self);

    g_signal_connect(self->leave_button, "clicked",
            G_CALLBACK(leave_button_on_click), self);
    g_signal_connect(self->show_topic_togglebutton, "clicked",
            G_CALLBACK(option_togglebutton_on_click), self->topic_revealer);
    g_signal_connect(self->show_user_list_togglebutton, "clicked",
            G_CALLBACK(option_togglebutton_on_click), self->user_list_revealer);

    g_signal_connect(self->upload_image_button, "clicked",
            G_CALLBACK(upload_image_button_on_click), self->input_entry);

    /* Create a tree model and use it as the completion model */
    gtk_entry_completion_set_model (self->entrycompletion,
            GTK_TREE_MODEL(self->completion_list));
    gtk_entry_completion_complete(self->entrycompletion);
    /* Use model column 0 as the text column */
    gtk_entry_completion_set_text_column (self->entrycompletion, 0);

    /* command completion */
    int i;
    for (i = 0; cmd_list[i] != 0; i++){
        srain_chan_completion_list_add(self, cmd_list[i]);
    }
}

static void srain_chan_class_init(SrainChanClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chan.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_label);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_list_box);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, user_list_revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, user_list_viewport);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, input_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, entrycompletion);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, option_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, show_topic_togglebutton);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, show_user_list_togglebutton);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, leave_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, upload_image_button);
}

SrainChan* srain_chan_new(const char *srv_name, const char *chan_name){
    SrainChan *chan;

    chan = g_object_new(SRAIN_TYPE_CHAN, NULL);

    gtk_label_set_text(chan->name_label, chan_name);
    gtk_widget_set_name(GTK_WIDGET(chan), chan_name);

    return chan;
}

void srain_chan_set_topic(SrainChan *chan, const char *topic){
    GString *markuped_topic;

    markuped_topic = markup(topic, NULL);
    if (markuped_topic){
        gtk_label_set_markup(chan->topic_label, markuped_topic->str);
        g_string_free(markuped_topic, TRUE);
    } else {
        gtk_label_set_text(chan->topic_label, topic);
    }
}

void srain_chan_fcous_entry(SrainChan *chan){
    gtk_widget_grab_focus(GTK_WIDGET(chan->input_entry));
}

SrainUserList* srain_chan_get_user_list(SrainChan *chan){
    if (SRAIN_IS_CHAN(chan)) {
        return chan->user_list;
    }

    return NULL;
}

SrainMsgList* srain_chan_get_msg_list(SrainChan *chan){
    if (SRAIN_IS_CHAN(chan)) {
        return chan->msg_list;
    }

    return NULL;
}

void srain_chan_completion_list_add(SrainChan *chan, const char *word){
  GtkTreeIter iter;

  gtk_list_store_append(chan->completion_list, &iter);
  gtk_list_store_set(chan->completion_list, &iter, 0, word, -1);
}

void srain_chan_completion_list_rm(SrainChan *chan, const char *word){
}
