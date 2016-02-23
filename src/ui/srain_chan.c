#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_msg.h"
#include "srain_detail_dialog.h"
#include "srain.h"
#include "log.h"

struct _SrainChan {
    GtkBox parent;
    GtkLabel* name_label;
    GtkLabel *topic_label;
    GtkListBox *msg_listbox;
    GtkRevealer *revealer;
    GtkButton *onlinelist_button;
    GtkListBox *online_listbox;
    GtkButton *send_button;
    GtkEntry *input_entry;
};

struct _SrainChanClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChan, srain_chan, GTK_TYPE_BOX);

static void onlinelist_button_on_click(GtkWidget *widget, gpointer *user_data){
    static gboolean is_show = FALSE;
    GtkImage *image;
    GtkRevealer *revealer;

    image = GTK_IMAGE(gtk_button_get_image(GTK_BUTTON(widget)));
    revealer = GTK_REVEALER(user_data);

    gtk_revealer_set_reveal_child(revealer, is_show = !is_show);
    gtk_image_set_from_icon_name(image, is_show ? "go-next":"go-previous", GTK_ICON_SIZE_BUTTON);
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
    g_signal_connect(self->onlinelist_button, "clicked", G_CALLBACK(onlinelist_button_on_click), self->revealer);
    g_signal_connect(self->online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);
}

static void srain_chan_class_init(SrainChanClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chan.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, topic_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChan, msg_listbox);
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

    return chan;
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

void srain_chan_sys_msg_add(SrainChan *chan, const char *msg){
    SrainSysMsg *smsg;

    smsg = srain_sys_msg_new(msg);
    gtk_widget_show(GTK_WIDGET(smsg));
    gtk_container_add(GTK_CONTAINER(chan->msg_listbox), GTK_WIDGET(smsg));
}

void srain_chan_send_msg_add(SrainChan *chan, const char *msg){
    SrainSendMsg *smsg;

    smsg = srain_send_msg_new(msg);
    gtk_widget_show(GTK_WIDGET(smsg));
    gtk_container_add(GTK_CONTAINER(chan->msg_listbox), GTK_WIDGET(smsg));
}

void srain_chan_recv_msg_add(SrainChan *chan, const char *nick, const char *id, const char *msg){
    SrainRecvMsg *smsg;

    smsg = srain_recv_msg_new(nick, id, msg);
    gtk_widget_show(GTK_WIDGET(smsg));
    gtk_container_add(GTK_CONTAINER(chan->msg_listbox), GTK_WIDGET(smsg));
}
