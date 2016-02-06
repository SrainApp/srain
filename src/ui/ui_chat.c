#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "log.h"
#include "srain.h"

/* extern variable from ui_window.c */
extern GtkWidget *chat_panel_stack;

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
            // replace it with a WHOIS function
            return TRUE;
        }
    }
    return FALSE;
}

static void send_button_on_click(GtkEntry *entry, GdkEventButton *event){
    const char *input;
    const char *panel;

    if(event->button == 1){
        panel = gtk_stack_get_visible_child_name(GTK_STACK(chat_panel_stack));
        input = gtk_entry_get_text(entry);
        LOG_FR("panel = %s, text = \"%s\"", panel, input);
        srain_send(panel, input);
    }
}

int ui_chat_add(const char *name, const char *topic){
    GtkBuilder *builder;
    GtkWidget *chat_name_label;
    GtkWidget *chat_topic_label;
    GtkWidget *chat_panel_box;
    GtkWidget *chat_online_listbox;
    GtkWidget *chat_send_button;
    GtkWidget *chat_input_entry;

    builder = gtk_builder_new_from_file("../ui/chat_panel.glade");
    UI_BUILDER_GET_WIDGET(builder, chat_panel_box);
    UI_BUILDER_GET_WIDGET(builder, chat_name_label);
    UI_BUILDER_GET_WIDGET(builder, chat_topic_label);
    UI_BUILDER_GET_WIDGET(builder, chat_online_listbox);
    UI_BUILDER_GET_WIDGET(builder, chat_send_button);
    UI_BUILDER_GET_WIDGET(builder, chat_input_entry);

    gtk_label_set_text(GTK_LABEL(chat_name_label), name);
    gtk_label_set_text(GTK_LABEL(chat_topic_label), topic);
    gtk_stack_add_named(GTK_STACK(chat_panel_stack), chat_panel_box, (gchar *)name);
    gtk_container_child_set(GTK_CONTAINER(chat_panel_stack), chat_panel_box, "title", (gchar *)name, NULL);

    g_signal_connect(chat_online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);
    g_signal_connect_swapped(chat_send_button, "button_release_event", G_CALLBACK(send_button_on_click), chat_input_entry);

    g_object_unref(G_OBJECT(builder));

    return 0;
}

int ui_chat_rm(const char *name){
    GtkWidget *chat_panel_box;

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), name);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s no found", name);
        return -1;
    }
    gtk_container_remove(GTK_CONTAINER(chat_panel_stack), chat_panel_box);

    return 0;
}

int ui_chat_set_topic(const char *name, const char *topic){
    GtkWidget *chat_topic_label;
    GtkWidget *chat_panel_box;

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), name);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s no found", name);
        return -1;
    }
    chat_topic_label = get_widget_by_name(chat_panel_box, "chat_topic_label");
    assert(chat_topic_label);

    gtk_label_set_text(GTK_LABEL(chat_topic_label), topic);

    return 0;
}

int ui_online_list_add(const char *chat_name, const char *nick){
    GtkWidget *nick_label;
    GtkWidget *chat_panel_box;
    GtkWidget *chat_online_listbox;
    GtkListBoxRow *item;

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), chat_name);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s no found", chat_name);
        return -1;
    }
    chat_online_listbox = get_widget_by_name(chat_panel_box, "chat_online_listbox");
    assert(chat_online_listbox);

    /* is this nick already exist? */
    item = get_list_item_by_name(GTK_LIST_BOX(chat_online_listbox), nick);
    if (item){
        ERR_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    nick_label = gtk_label_new(nick);
    gtk_widget_set_name(nick_label, nick);

    gtk_container_add(GTK_CONTAINER(chat_online_listbox), nick_label);
    gtk_widget_show(nick_label);

    return 0;
}

int ui_online_list_rm(const char *chat_name, const char *nick){
    GtkWidget *chat_panel_box;
    GtkListBox *chat_online_listbox;
    GtkListBoxRow *item;

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), chat_name);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s no found", chat_name);
        return -1;
    }
    chat_online_listbox= GTK_LIST_BOX(get_widget_by_name(chat_panel_box, "chat_online_listbox"));
    assert(chat_online_listbox);

    item = get_list_item_by_name(chat_online_listbox, nick);
    if (!item){
        ERR_FR("GtkListBoxRow %s no found", nick);
        return -1;
    }

    gtk_container_remove(GTK_CONTAINER(chat_online_listbox), GTK_WIDGET(item));

    return 0;
}
