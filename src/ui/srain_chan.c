#include <gtk/gtk.h>
#include <assert.h>
#include "ui.h"
#include "ui_common.h"
#include "log.h"
#include "srain.h"
#include "srain_window.h"
#include "srain_chan.h"

struct _SrainChan {
    GtkBox parent;
    GtkLabel* name_label;
    GtkLabel *topic_label;
    GtkListBox *msg_listbox;
    GtkListBox *online_listbox;
    GtkButton *send_button;
    GtkEntry *input_entry;
};

struct _SrainChanClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChan, srain_chan, GTK_TYPE_BOX);

static gint online_listbox_on_dbclick(GtkWidget *widget, GdkEventButton *event){
    const char *nick;
    GtkWidget *label;
    GtkListBoxRow *row;

    if(event->button == 1 && event->type == GDK_2BUTTON_PRESS){
        row = gtk_list_box_get_selected_row(GTK_LIST_BOX(widget));
        if (row){
            label = gtk_bin_get_child(GTK_BIN(row));
            nick = gtk_label_get_text(GTK_LABEL(label));
            detail_dialog_init(nick, "");
            // replace it with a WHOIS function return TRUE;
        }
    }
    return FALSE;
}

static void on_send(SrainChan *chan){
    int res;
    const char *input;

    assert(SRAIN_IS_CHAN(chan));

    input = gtk_entry_get_text(chan->input_entry);
    LOG_FR("panel = %s, text = \"%s\"", gtk_widget_get_name(GTK_WIDGET(chan)), input);
    if (input[0] == '/'){
        // res = srain_cmd(panel, input);
    } else {
        // res = srain_send(panel, input);
    }
    if (res != -1)
        gtk_entry_set_text(chan->input_entry, "");
}

static void srain_chan_init(SrainChan *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect_swapped(self->input_entry, "activate", G_CALLBACK(on_send), self);
    g_signal_connect_swapped(self->send_button, "clicked", G_CALLBACK(on_send), self);
    g_signal_connect(self->online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);

}

static void srain_chan_class_init(SrainChanClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chan.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_listbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, online_listbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, send_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, input_entry);
}

SrainChan* srain_chan_new(const char *name){
    return g_object_new(SRAIN_TYPE_CHAN, NULL);
}

void srain_chan_set_name(SrainChan *chan, const char *name){
    gtk_label_set_text(chan->name_label, name);
}

void srain_chan_set_topic(SrainChan *chan, const char *topic){
    gtk_label_set_text(chan->topic_label, topic);
}

void srain_chan_online_list_add(SrainChan *chan, const char *name){
    GtkWidget *label;
    GtkListBoxRow *row;

    row = get_list_item_by_name(chan->online_listbox, name);
    if (row){
        ERR_FR("GtkListBoxRow %s already exist", name);
        return;
    }
    label = gtk_label_new(name);
    gtk_widget_set_name(label, name);

    gtk_container_add(GTK_CONTAINER(chan->online_listbox), label);
    gtk_widget_show(label);
}

void srain_chan_online_list_rm(SrainChan *chan, const char *name){
    GtkListBoxRow *row;

    row = get_list_item_by_name(chan->online_listbox, name);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found", name);
        return;
    }
    gtk_container_remove(GTK_CONTAINER(chan->online_listbox), GTK_WIDGET(row));
}
