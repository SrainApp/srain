#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"

/* extern variable from ui_window.c */
extern GtkWidget *chat_panel_stack;

int ui_chat_add(const char *name, const char *topic){
    GtkBuilder *builder;
    GtkWidget *chat_name_label;
    GtkWidget *chat_topic_label;
    GtkWidget *chat_panel_box;
    GtkWidget *chat_msg_listbox;

    builder = gtk_builder_new_from_file("../ui/chat_panel.glade");
    UI_BUILDER_GET_WIDGET(builder, chat_panel_box);
    UI_BUILDER_GET_WIDGET(builder, chat_msg_listbox);
    UI_BUILDER_GET_WIDGET(builder, chat_name_label);
    UI_BUILDER_GET_WIDGET(builder, chat_topic_label);

    gtk_label_set_text(GTK_LABEL(chat_name_label), name);
    gtk_label_set_text(GTK_LABEL(chat_topic_label), topic);
    gtk_stack_add_named(GTK_STACK(chat_panel_stack), chat_panel_box, (gchar *)name);
    gtk_container_child_set(GTK_CONTAINER(chat_panel_stack), chat_panel_box, "title", (gchar *)name, NULL);

    g_object_unref(G_OBJECT(builder));

    return 1;
}

int ui_chat_rm(const char *name){
    GtkWidget *chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), name);
    if (!chat_panel_box) return 0;
    gtk_container_remove(GTK_CONTAINER(chat_panel_stack), chat_panel_box);
    return 1;
}

void ui_chat_set_topic(const char *name, const char *topic){
    GtkWidget *chat_topic_label;
    chat_topic_label = get_widget_by_name(gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), name), "chat_topic_label");
    assert(chat_topic_label);
    gtk_label_set_text(GTK_LABEL(chat_topic_label), topic);
}

int ui_online_list_add(const char *chat_name, const char *nick){
    GtkWidget *nick_label;
    GtkWidget *chat_online_listbox;

    nick_label = gtk_label_new(nick);
    gtk_widget_set_name(nick_label, nick);

    chat_online_listbox = get_widget_by_name(gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), chat_name), "chat_online_listbox");
    assert(chat_online_listbox);
    gtk_container_add(GTK_CONTAINER(chat_online_listbox), nick_label);
    gtk_widget_show(nick_label);

    return 1;
}

int ui_online_list_rm(const char *chat_name, const char *nick){
    GtkListBox *listbox;
    GtkListBoxRow*label;

    listbox = GTK_LIST_BOX(get_widget_by_name(gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), chat_name), "chat_online_listbox"));
    label = get_list_item_by_name(listbox, nick);
    gtk_container_remove(GTK_CONTAINER(listbox), GTK_WIDGET(label));

    return 1;
}
