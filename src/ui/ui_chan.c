#include <gtk/gtk.h>
#include <assert.h>
#include "ui.h"
#include "ui_common.h"
#include "log.h"
#include "srain.h"

/* extern variable from ui_window.c */
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

gboolean ui_chan_add(chan_name_t *chan){
    GtkBuilder *builder;
    GtkWidget *chan_name_label;
    GtkWidget *chan_topic_label;
    GtkWidget *chan_panel_box;
    GtkWidget *chan_online_listbox;
    GtkWidget *chan_send_button;
    GtkWidget *chan_input_entry;

    assert(chan);
    LOG_FR("add chan %s", chan->name);

    builder = gtk_builder_new_from_file("../data/ui/chan_panel.glade");
    UI_BUILDER_GET_WIDGET(builder, chan_panel_box);
    UI_BUILDER_GET_WIDGET(builder, chan_name_label);
    UI_BUILDER_GET_WIDGET(builder, chan_topic_label);
    UI_BUILDER_GET_WIDGET(builder, chan_online_listbox);
    UI_BUILDER_GET_WIDGET(builder, chan_send_button);
    UI_BUILDER_GET_WIDGET(builder, chan_input_entry);

    gtk_label_set_text(GTK_LABEL(chan_name_label), chan->name);
    gtk_stack_add_named(GTK_STACK(chan_panel_stack), chan_panel_box, chan->name);
    gtk_container_child_set(GTK_CONTAINER(chan_panel_stack), chan_panel_box, "title", chan->name, NULL);

    g_signal_connect(chan_online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);
    g_signal_connect(chan_input_entry, "key_press_event", G_CALLBACK(input_entry_on_enter), NULL);
    g_signal_connect_swapped(chan_send_button, "button_release_event", G_CALLBACK(send_button_on_click), chan_input_entry);

    g_object_unref(G_OBJECT(builder));
    free(chan);

    return FALSE;
}

gboolean ui_chan_rm(chan_name_t *chan){
    GtkWidget *chan_panel_box;

    assert(chan);

    chan_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chan_panel_stack), chan->name);
    if (!chan_panel_box){
        ERR_FR("chan_panel %s no found", chan->name);
        goto bad;
    }
    gtk_container_remove(GTK_CONTAINER(chan_panel_stack), chan_panel_box);

    free(chan);
    return FALSE;

bad:
    free(chan);
    return FALSE;
}

const char* ui_chan_get_cur(){
    return gtk_stack_get_visible_child_name(GTK_STACK(chan_panel_stack));
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
