#include <gtk/gtk.h>
#include <assert.h>
#include "ui.h"
#include "ui_common.h"
#include "log.h"
#include "srain.h"
#include "srain_window.h"
#include "srain_chan_box.h"

struct _SrainChanBox {
    GtkBox parent;
    GtkWidget *name_label;
    GtkWidget *topic_label;
    GtkWidget *msg_listbox;
    GtkWidget *online_listbox;
    GtkWidget *send_button;
    GtkWidget *input_entry;
};

struct _SrainChanBoxClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainChanBox, srain_chan_box, GTK_TYPE_BOX);

static void srain_chan_box_init(SrainChanBox *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_chan_box_class_init(SrainChanBoxClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/chan_box.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChanBox, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChanBox, topic_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChanBox, msg_listbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChanBox, online_listbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChanBox, send_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainChanBox, input_entry);
}

SrainChanBox* srain_chan_box_new(const char *name){
    return g_object_new(SRAIN_TYPE_CHAN_BOX, NULL);
}

/* extern variable from ui_chanbox.c */
extern GtkWidget *chan_panel_stack;

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

static gint input_entry_on_enter(GtkWidget *widget, GdkEventKey *event){
    int res = 0;
    const char *input;
    const char *panel;

    if (event->keyval == GDK_KEY_Return){
        panel = gtk_stack_get_visible_child_name(GTK_STACK(chan_panel_stack));
        assert(panel);
        input = gtk_entry_get_text(GTK_ENTRY(widget));
        LOG_FR("panel = %s, text = \"%s\"", panel, input);
        if (input[0] == '/'){
            res = srain_cmd(panel, input);
        } else {
            res = srain_send(panel, input);
        }
        if (res != -1){
            gtk_entry_set_text(GTK_ENTRY(widget), "");
        }
        return TRUE;
    }
    return FALSE;
}

static void send_button_on_click(GtkEntry *entry, GdkEventButton *event){
    const char *input;
    const char *panel;

    if(event->button == 1){
        panel = gtk_stack_get_visible_child_name(GTK_STACK(chan_panel_stack));
        assert(panel);
        input = gtk_entry_get_text(entry);
        LOG_FR("panel = %s, text = \"%s\"", panel, input);
        if (srain_send(panel, input) != -1){
            gtk_entry_set_text(entry, "");
        }
    }
}

int ui_chan_add(const char *chan){
    GtkBuilder *builder;
    GtkWidget *chan_name_label;
    GtkWidget *chan_topic_label;
    GtkWidget *chan_panel_box;
    GtkWidget *chan_online_listbox;
    GtkWidget *chan_send_button;
    GtkWidget *chan_input_entry;

    LOG_FR("add chan %s", chan);

    builder = gtk_builder_new_from_file("../data/ui/chan_panel.glade");
    UI_BUILDER_GET_WIDGET(builder, chan_panel_box);
    UI_BUILDER_GET_WIDGET(builder, chan_name_label);
    UI_BUILDER_GET_WIDGET(builder, chan_topic_label);
    UI_BUILDER_GET_WIDGET(builder, chan_online_listbox);
    UI_BUILDER_GET_WIDGET(builder, chan_send_button);
    UI_BUILDER_GET_WIDGET(builder, chan_input_entry);

    gtk_label_set_text(GTK_LABEL(chan_name_label), chan);
    gtk_stack_add_named(GTK_STACK(chan_panel_stack), chan_panel_box, chan);
    gtk_container_child_set(GTK_CONTAINER(chan_panel_stack), chan_panel_box, "title", chan, NULL);

    g_signal_connect(chan_online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);
    g_signal_connect(chan_input_entry, "key_press_event", G_CALLBACK(input_entry_on_enter), NULL);
    g_signal_connect_swapped(chan_send_button, "button_release_event", G_CALLBACK(send_button_on_click), chan_input_entry);

    g_object_unref(G_OBJECT(builder));

    return FALSE;
}

int ui_chan_rm(const char *chan){
    GtkWidget *chan_panel_box;

    chan_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chan_panel_stack), chan);
    if (!chan_panel_box){
        ERR_FR("chan_panel %s no found", chan);
        return FALSE;
    }
    gtk_container_remove(GTK_CONTAINER(chan_panel_stack), chan_panel_box);

    return FALSE;
}

gboolean ui_chan_set_topic(topic_t *topic){
    GtkWidget *chan_topic_label;
    GtkWidget *chan_panel_box;

    assert(topic);

    chan_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chan_panel_stack), topic->chan);
    if (!chan_panel_box){
        ERR_FR("chan_panel %s no found", topic->chan);
        goto bad;
    }
    chan_topic_label = get_widget_by_name(chan_panel_box, "chan_topic_label");
    assert(chan_topic_label);

    gtk_label_set_text(GTK_LABEL(chan_topic_label), topic->topic);

    free(topic);
    return FALSE;

bad:
    free(topic);
    return FALSE;
}

gboolean ui_online_list_add(online_list_item_t *item){
    GtkWidget *nick_label;
    GtkWidget *chan_panel_box;
    GtkWidget *chan_online_listbox;
    GtkListBoxRow *row;

    assert(item);

    chan_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chan_panel_stack), item->chan);
    if (!chan_panel_box){
        ERR_FR("chan_panel %s no found", item->chan);
        goto bad;
    }
    chan_online_listbox = get_widget_by_name(chan_panel_box, "chan_online_listbox");
    assert(chan_online_listbox);

    /* is this nick already exist? */
    row = get_list_item_by_name(GTK_LIST_BOX(chan_online_listbox), item->nick);
    if (row){
        ERR_FR("GtkListBoxRow %s already exist", item->nick);
        goto bad;
    }

    nick_label = gtk_label_new(item->nick);
    gtk_widget_set_name(nick_label, item->nick);

    gtk_container_add(GTK_CONTAINER(chan_online_listbox), nick_label);
    gtk_widget_show(nick_label);

    free(item);
    return FALSE;

bad:
    free(item);
    return FALSE;
}

int ui_online_list_rm(online_list_item_t *item){
    GtkWidget *chan_panel_box;
    GtkListBox *chan_online_listbox;
    GtkListBoxRow *row;

    assert(item);

    chan_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chan_panel_stack), item->chan);
    if (!chan_panel_box){
        ERR_FR("chan_panel %s no found", item->nick);
        goto bad;
    }
    chan_online_listbox= GTK_LIST_BOX(get_widget_by_name(chan_panel_box, "chan_online_listbox"));
    assert(chan_online_listbox);

    row = get_list_item_by_name(chan_online_listbox, item->nick);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found", item->nick);
        goto bad;
    }

    gtk_container_remove(GTK_CONTAINER(chan_online_listbox), GTK_WIDGET(row));

    free(item);
    return FALSE;

bad:
    free(item);
    return FALSE;
}

const char* ui_chan_get_cur(){
    return gtk_stack_get_visible_child_name(GTK_STACK(chan_panel_stack));
}

