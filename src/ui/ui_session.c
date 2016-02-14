#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "log.h"
#include "srain.h"

/* extern variable from ui_window.c */
extern GtkWidget *session_panel_stack;

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
    const char *input;
    const char *panel;

    if (event->keyval == GDK_KEY_Return){
        panel = gtk_stack_get_visible_child_name(GTK_STACK(session_panel_stack));
        assert(panel);
        input = gtk_entry_get_text(GTK_ENTRY(widget));
        LOG_FR("panel = %s, text = \"%s\"", panel, input);
        if (srain_send(panel, input) != -1){
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
        panel = gtk_stack_get_visible_child_name(GTK_STACK(session_panel_stack));
        assert(panel);
        input = gtk_entry_get_text(entry);
        LOG_FR("panel = %s, text = \"%s\"", panel, input);
        if (srain_send(panel, input) != -1){
            gtk_entry_set_text(entry, "");
        }
    }
}

int ui_session_add(const char *name){
    GtkBuilder *builder;
    GtkWidget *session_name_label;
    GtkWidget *session_topic_label;
    GtkWidget *session_panel_box;
    GtkWidget *session_online_listbox;
    GtkWidget *session_send_button;
    GtkWidget *session_input_entry;

    builder = gtk_builder_new_from_file("../data/ui/session_panel.glade");
    UI_BUILDER_GET_WIDGET(builder, session_panel_box);
    UI_BUILDER_GET_WIDGET(builder, session_name_label);
    UI_BUILDER_GET_WIDGET(builder, session_topic_label);
    UI_BUILDER_GET_WIDGET(builder, session_online_listbox);
    UI_BUILDER_GET_WIDGET(builder, session_send_button);
    UI_BUILDER_GET_WIDGET(builder, session_input_entry);

    gtk_label_set_text(GTK_LABEL(session_name_label), name);
    gtk_stack_add_named(GTK_STACK(session_panel_stack), session_panel_box, (gchar *)name);
    gtk_container_child_set(GTK_CONTAINER(session_panel_stack), session_panel_box, "title", (gchar *)name, NULL);

    g_signal_connect(session_online_listbox, "button_press_event", G_CALLBACK(online_listbox_on_dbclick), NULL);
    g_signal_connect(session_input_entry, "key_press_event", G_CALLBACK(input_entry_on_enter), NULL);
    g_signal_connect_swapped(session_send_button, "button_release_event", G_CALLBACK(send_button_on_click), session_input_entry);

    g_object_unref(G_OBJECT(builder));

    return 0;
}

int ui_session_rm(const char *name){
    GtkWidget *session_panel_box;

    session_panel_box = gtk_stack_get_child_by_name(GTK_STACK(session_panel_stack), name);
    if (!session_panel_box){
        ERR_FR("session_panel %s no found", name);
        return -1;
    }
    gtk_container_remove(GTK_CONTAINER(session_panel_stack), session_panel_box);

    return 0;
}

int ui_session_set_topic(const char *name, const char *topic){
    GtkWidget *session_topic_label;
    GtkWidget *session_panel_box;

    session_panel_box = gtk_stack_get_child_by_name(GTK_STACK(session_panel_stack), name);
    if (!session_panel_box){
        ERR_FR("session_panel %s no found", name);
        return -1;
    }
    session_topic_label = get_widget_by_name(session_panel_box, "session_topic_label");
    assert(session_topic_label);

    gtk_label_set_text(GTK_LABEL(session_topic_label), topic);

    return 0;
}

int ui_online_list_add(const char *session_name, const char *nick){
    GtkWidget *nick_label;
    GtkWidget *session_panel_box;
    GtkWidget *session_online_listbox;
    GtkListBoxRow *item;

    session_panel_box = gtk_stack_get_child_by_name(GTK_STACK(session_panel_stack), session_name);
    if (!session_panel_box){
        ERR_FR("session_panel %s no found", session_name);
        return -1;
    }
    session_online_listbox = get_widget_by_name(session_panel_box, "session_online_listbox");
    assert(session_online_listbox);

    /* is this nick already exist? */
    item = get_list_item_by_name(GTK_LIST_BOX(session_online_listbox), nick);
    if (item){
        ERR_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    nick_label = gtk_label_new(nick);
    gtk_widget_set_name(nick_label, nick);

    gtk_container_add(GTK_CONTAINER(session_online_listbox), nick_label);
    gtk_widget_show(nick_label);

    return 0;
}

int ui_online_list_rm(const char *session_name, const char *nick){
    GtkWidget *session_panel_box;
    GtkListBox *session_online_listbox;
    GtkListBoxRow *item;

    session_panel_box = gtk_stack_get_child_by_name(GTK_STACK(session_panel_stack), session_name);
    if (!session_panel_box){
        ERR_FR("session_panel %s no found", session_name);
        return -1;
    }
    session_online_listbox= GTK_LIST_BOX(get_widget_by_name(session_panel_box, "session_online_listbox"));
    assert(session_online_listbox);

    item = get_list_item_by_name(session_online_listbox, nick);
    if (!item){
        ERR_FR("GtkListBoxRow %s no found", nick);
        return -1;
    }

    gtk_container_remove(GTK_CONTAINER(session_online_listbox), GTK_WIDGET(item));

    return 0;
}
