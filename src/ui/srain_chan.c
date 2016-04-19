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
#include "ui_common.h"
#include "theme.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_user_list.h"
#include "srain_msg.h"
#include "markup.h"
#include "plugin.h"
#include "log.h"
#include "irc.h"
#include "irc_magic.h"

struct _SrainChan {
    GtkBox parent;

    /* header */
    GtkLabel* name_label;
    GtkButton *option_button;
    GtkRevealer *topic_revealer;
    GtkLabel *topic_label;

    /* option box */
    GtkPopover *option_popover;
    GtkBox *option_box;
    GtkToggleButton *show_topic_togglebutton;
    GtkToggleButton *show_user_list_togglebutton;
    GtkButton *leave_button;

    /* */
    GtkScrolledWindow *msg_scrolledwindow;
    GtkListBox *msg_listbox;
    GtkMenu *msg_menu;
    GtkRevealer *user_list_revealer;
    GtkViewport *user_list_viewport;
    SrainUserList *user_list;

    /* annex grid */
    GtkButton *annex_button;
    GtkGrid *annex_grid;
    GtkPopover *annex_popover;
    GtkButton *annex_image_button;
    // GtkButton *annex_shot_button;

    /* input entry */
    GtkEntry *input_entry;
    GtkEntryCompletion *entrycompletion;
    GtkListStore *completion_list;

    GtkWidget *last_msg;
    // GtkLabel *unread_label;
};

struct _SrainChanClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChan, srain_chan, GTK_TYPE_BOX);

static void srain_chan_scroll_up(SrainChan *chan){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(chan->msg_scrolledwindow);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) - 30);
    gtk_scrolled_window_set_vadjustment(chan->msg_scrolledwindow, adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

static void srain_chan_scroll_down(SrainChan *chan){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(chan->msg_scrolledwindow);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) + 30);
    gtk_scrolled_window_set_vadjustment(chan->msg_scrolledwindow, adj);

    while (gtk_events_pending()) gtk_main_iteration();
}

static gboolean scroll_to_bottom(SrainChan *chan){
    GtkAdjustment *adj;
    double val;
    double max_val;

    adj = gtk_scrolled_window_get_vadjustment(chan->msg_scrolledwindow);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) -
            gtk_adjustment_get_page_size(adj));
    gtk_scrolled_window_set_vadjustment(chan->msg_scrolledwindow, adj);

    gtk_widget_queue_draw(GTK_WIDGET(chan));

    adj = gtk_scrolled_window_get_vadjustment(chan->msg_scrolledwindow);
    val = gtk_adjustment_get_value(adj);
    max_val = gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj);
    // LOG_FR("cur val: %f, max val %f", val, max_val);

    if (max_val - val > 10) {
        LOG_FR("retry");
        return TRUE;
    }

    return FALSE;
}

// TODO: ugly & ambiguous code
static int should_scroll_to_bottom(SrainChan *chan){
    double val;
    double max_val;
    SrainWindow *win;
    SrainChan *cur_chan;
    GtkAdjustment *adj;

    win = SRAIN_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(chan)));
    cur_chan = srain_window_get_cur_chan(win);
    adj = gtk_scrolled_window_get_vadjustment(chan->msg_scrolledwindow);

    val = gtk_adjustment_get_value(adj);
    max_val = gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj);
    // LOG_FR("cur val: %f, max val %f", val, max_val);

    // if (gtk_window_is_active(GTK_WINDOW(win))
    if (cur_chan == chan && max_val - val < 10){
        // LOG_FR("you should go to bottom!");
        return 1;
    }
    /*
       if (chan->unread_label == NULL){
       chan->unread_label = GTK_LABEL(gtk_label_new("Unread messages"));
       gtk_widget_set_name(GTK_WIDGET(chan->unread_label), "unread_label");
       gtk_container_add(GTK_CONTAINER(chan->msg_listbox),
       GTK_WIDGET(chan->unread_label));
       gtk_widget_show(GTK_WIDGET(chan->unread_label));
       }
    */

    return 0;
}

void completion_list_add(GtkListStore *store, const char *word){
  GtkTreeIter iter;

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, word, -1);
}

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
            srain_chan_scroll_down(chan);
            break;
        case GDK_KEY_Up:
            srain_chan_scroll_up(chan);
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

static void annex_image_button_on_click(GtkWidget *widget, gpointer user_data){
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
    const char *chan_name;
    SrainChan *chan;

    chan = user_data;
    // TODO: unquery
    // chan_name = gtk_widget_get_name(GTK_WIDGET(chan));
    srain_app_part(chan);
}

static void option_togglebutton_on_click(GtkWidget *widget, gpointer user_data){
    GtkRevealer *revealer;
    GtkToggleButton *button;

    revealer = user_data;
    button = GTK_TOGGLE_BUTTON(widget);
    gtk_revealer_set_reveal_child(revealer,
            gtk_toggle_button_get_active(button));
}

static void popover_show(gpointer user_data){
    GtkPopover *popover;

    popover = user_data;
    gtk_widget_set_visible(GTK_WIDGET(popover), TRUE);
}

/* leave_buttons, option_togglebutton on click */
static void popover_hide(gpointer user_data){
    GtkPopover *popover;

    popover = user_data;
    gtk_widget_set_visible(GTK_WIDGET(popover), FALSE);
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

    LOG_FR("panel = %s, text = '%s'", chan_name, input);

    if (input[0] == '/'){
        srain_app_cmd(chan, input);
    } else {
        srain_app_send(chan, input);
    }

ret:
    gtk_entry_set_text(chan->input_entry, "");
    free(input);
    return;
}

// TODO: NOT work now
static gboolean msg_listbox_popup(GtkWidget *widget,
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
    self->completion_list = gtk_list_store_new(1, G_TYPE_STRING);

    /* init option popover*/
    self->option_popover = create_popover(GTK_WIDGET(self->option_button),
            GTK_WIDGET(self->option_box), GTK_POS_BOTTOM);
    gtk_container_set_border_width(GTK_CONTAINER(self->option_popover), 6);

    /* init annex popover*/
    self->annex_popover = create_popover(GTK_WIDGET(self->annex_button),
            GTK_WIDGET(self->annex_grid), GTK_POS_TOP);
    gtk_container_set_border_width(GTK_CONTAINER(self->annex_popover), 6);

    /* init user list */
    self->user_list = srain_user_list_new();
    gtk_container_add(GTK_CONTAINER(self->user_list_viewport),
            GTK_WIDGET(self->user_list));
    gtk_widget_show(GTK_WIDGET(self->user_list));

    self->last_msg = NULL;

    theme_apply(GTK_WIDGET(self->msg_menu));

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
    g_signal_connect_swapped(self->option_button, "clicked",
            G_CALLBACK(popover_show), self->option_popover);
    g_signal_connect_swapped(self->leave_button, "clicked",
            G_CALLBACK(popover_hide), self->option_popover);
    g_signal_connect_swapped(self->show_topic_togglebutton, "clicked",
            G_CALLBACK(popover_hide), self->option_popover);
    g_signal_connect_swapped(self->show_user_list_togglebutton, "clicked",
            G_CALLBACK(popover_hide), self->option_popover);

    // g_signal_connect(self->user_list_listbox, "button_press_event",
            // G_CALLBACK(online_listbox_on_dbclick), NULL);
    g_signal_connect(self->msg_listbox, "button_press_event",
            G_CALLBACK(msg_listbox_popup), self->msg_menu);

    g_signal_connect_swapped(self->annex_button, "clicked",
            G_CALLBACK(popover_show), self->annex_popover);
    g_signal_connect(self->annex_image_button, "clicked",
            G_CALLBACK(annex_image_button_on_click), self->input_entry);
    g_signal_connect_swapped(self->annex_image_button, "clicked",
            G_CALLBACK(popover_hide), self->annex_popover);

    /* Create a tree model and use it as the completion model */
    gtk_entry_completion_set_model (self->entrycompletion,
            GTK_TREE_MODEL(self->completion_list));
    gtk_entry_completion_complete(self->entrycompletion);
    /* Use model column 0 as the text column */
    gtk_entry_completion_set_text_column (self->entrycompletion, 0);
}

static void srain_chan_class_init(SrainChanClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chan.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, option_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_scrolledwindow);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_listbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, user_list_revealer);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, user_list_viewport);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, input_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, entrycompletion);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, option_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, show_topic_togglebutton);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, show_user_list_togglebutton);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, leave_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, annex_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, annex_grid);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, annex_image_button);
    // gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, annex_shot_button);
}

SrainChan* srain_chan_new(const char *server_name, const char *chan_name){
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

void srain_chan_sys_msg_add(SrainChan *chan, SysMsgType type, const char *msg){
    int to_bottom;
    SrainSysMsg *smsg;

    smsg = srain_sys_msg_new(type, msg);

    to_bottom = (should_scroll_to_bottom(chan));

    gtk_list_box_add_unfocusable_row(chan->msg_listbox, GTK_WIDGET(smsg));

    chan->last_msg = GTK_WIDGET(smsg);

    if (to_bottom) gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, chan);
}


void srain_chan_sys_msg_addf(SrainChan *chan,
        SysMsgType type, const char *fmt, ...){
    char msg[512];
    va_list args;

    if (strlen(fmt) != 0 ){
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);

        srain_chan_sys_msg_add(chan, type, msg);
    }
}

void srain_chan_send_msg_add(SrainChan *chan, const char *msg){
    SrainSendMsg *smsg;

    smsg = srain_send_msg_new(msg);
    gtk_list_box_add_unfocusable_row(chan->msg_listbox, GTK_WIDGET(smsg));

    chan->last_msg = GTK_WIDGET(smsg);

    gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, chan);
}

void _srain_chan_recv_msg_add(SrainChan *chan, const char *nick, const char *id, const char *msg){
    SrainRecvMsg *smsg;

    smsg = srain_recv_msg_new(nick, id, msg);
    gtk_list_box_add_unfocusable_row(chan->msg_listbox, GTK_WIDGET(smsg));

    chan->last_msg = GTK_WIDGET(smsg);
}

/* add a SrainRecvMsg into SrainChan, if its time is same to the last msg, combine them */
void srain_chan_recv_msg_add(SrainChan *chan, const char *nick,
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

    to_bottom = (should_scroll_to_bottom(chan));

    if (chan->last_msg && SRAIN_IS_RECV_MSG(chan->last_msg)){
        last_recv_msg = SRAIN_RECV_MSG(chan->last_msg);
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
            if (to_bottom) gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, chan);

            return;
        }
    }

    _srain_chan_recv_msg_add(chan, nick, id, msg);

    if (to_bottom) gdk_threads_add_idle((GSourceFunc)scroll_to_bottom, chan);
}

void srain_chan_fcous_entry(SrainChan *chan){
    gtk_widget_grab_focus(GTK_WIDGET(chan->input_entry));
}

/**
 * @brief srain_chan_user_list_add
 *
 * @param chan
 * @param nick
 * @param is_init if is_init = 1, sys msg will not be sent
 */
void srain_chan_user_list_add(SrainChan *chan, const char *nick,
        IRCUserType type, int if_sys_msg){
    const char *chan_name;

    if (srain_user_list_add(chan->user_list, nick, type) != -1){
        completion_list_add(chan->completion_list, nick);

        chan_name = gtk_widget_get_name(GTK_WIDGET(chan));
        if (if_sys_msg)
            srain_chan_sys_msg_addf(chan, SYS_MSG_NORMAL, "%s has joined %s",
                    nick, chan_name);
    }
}

void srain_chan_user_list_rm(SrainChan *chan, const char *nick, const char *reason){
    const char *chan_name;

    if (srain_user_list_rm(chan->user_list, nick) != -1){
        chan_name =gtk_widget_get_name(GTK_WIDGET(chan));
        srain_chan_sys_msg_addf(chan, SYS_MSG_NORMAL, "%s has left %s: %s",
                nick, chan_name, reason);
    }
}
