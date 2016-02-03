#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include "ui_common.h"
#include "msg.h"

/* extern variable from ui_window.c */
extern GtkWidget *chat_panel_stack;

static gint nick_button_on_click(GtkWidget *widget, GdkEventButton *event, GtkLabel *label){
    const gchar *nick = gtk_label_get_text(label);
    if (event->button == 1){
        detail_dialog_init(nick, "");
        // replace it with a WHOIS function
        return TRUE;
    }
    return FALSE;
}

static gint msg_label_popup_handler(GtkWidget *label, GdkEventButton *event, GtkWidget *menu){
    if (event->button == 3 && !gtk_label_get_selection_bounds(GTK_LABEL(label), NULL, NULL)){
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

void ui_msg_send(const MsgSend msg){
    GtkBuilder *builder;
    GtkWidget *send_msg_bubble_box;
    GtkWidget *send_msg_label;
    GtkWidget *send_image;
    GtkWidget *send_time_label;
    GtkWidget *msg_bubble_menu;

    builder = gtk_builder_new_from_file( "../ui/msg_bubble.glade");
    // if (msg.img) UI_BUILDER_GET_WIDGET(builder, send_image);
    UI_BUILDER_GET_WIDGET(builder, send_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, send_msg_label);
    UI_BUILDER_GET_WIDGET(builder, send_time_label);
    UI_BUILDER_GET_WIDGET(builder, msg_bubble_menu);

    gtk_label_set_text(GTK_LABEL(send_msg_label), msg.msg);
    gtk_label_set_text(GTK_LABEL(send_time_label), msg.time);

    /* popmenu event of message label */
    g_signal_connect(G_OBJECT(send_msg_label), "button_press_event", G_CALLBACK(msg_label_popup_handler), G_OBJECT(msg_bubble_menu));
    g_object_ref(msg_bubble_menu); // TODO with out this statement, gtkmenu will be free after unref builder

    /* add msg_bubble into message listbox */
    GtkWidget *chat_msg_listbox = get_widget_by_name(gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), msg.chan), "chat_msg_listbox");
    assert(chat_msg_listbox);
    gtk_container_add(GTK_CONTAINER(chat_msg_listbox), send_msg_bubble_box);

    g_object_unref(G_OBJECT(builder));
}

void ui_msg_recv(const MsgRecv msg){
    GtkBuilder *builder;
    GtkWidget *recv_msg_bubble_box;
    GtkWidget *avatar_image;
    GtkWidget *recv_image;
    GtkWidget *nick_label;
    GtkWidget *nick_button;
    GtkWidget *identify_label;
    GtkWidget *recv_msg_label;
    GtkWidget *recv_time_label;
    GtkWidget *msg_bubble_menu;

    builder = gtk_builder_new_from_file( "../ui/msg_bubble.glade");
    if (msg.avatar) UI_BUILDER_GET_WIDGET(builder, avatar_image);
    if (msg.img) UI_BUILDER_GET_WIDGET(builder, recv_image);
    UI_BUILDER_GET_WIDGET(builder, recv_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, nick_label);
    UI_BUILDER_GET_WIDGET(builder, nick_button);
    UI_BUILDER_GET_WIDGET(builder, identify_label);
    UI_BUILDER_GET_WIDGET(builder, recv_msg_label);
    UI_BUILDER_GET_WIDGET(builder, recv_time_label);
    UI_BUILDER_GET_WIDGET(builder, msg_bubble_menu);

    gtk_label_set_text(GTK_LABEL(nick_label), msg.nick);
    gtk_label_set_text(GTK_LABEL(identify_label), msg.id);
    gtk_label_set_text(GTK_LABEL(recv_time_label), msg.time);
    gtk_label_set_text(GTK_LABEL(recv_msg_label), msg.msg);

    g_signal_connect(G_OBJECT(nick_button), "button_press_event", G_CALLBACK(nick_button_on_click), nick_label);
    g_signal_connect(G_OBJECT(recv_msg_label), "button_press_event", G_CALLBACK(msg_label_popup_handler), G_OBJECT(msg_bubble_menu));
    g_object_ref(msg_bubble_menu); // TODO with out this statement, gtkmenu will be free after unref builder

    /* add msg_bubble into message listbox */
    GtkWidget *chat_msg_listbox = get_widget_by_name(gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), msg.chan), "chat_msg_listbox");
    assert(chat_msg_listbox);
    gtk_container_add(GTK_CONTAINER(chat_msg_listbox), recv_msg_bubble_box);

    g_object_unref(G_OBJECT(builder));
}

void ui_msg_sys(const MsgSys msg){
    GtkBuilder *builder;
    GtkWidget *sys_msg_label;

    builder = gtk_builder_new_from_file( "../ui/msg_bubble.glade");
    UI_BUILDER_GET_WIDGET(builder, sys_msg_label);

    gtk_label_set_text(GTK_LABEL(sys_msg_label), msg.msg);

    /* add sys_msg_label into message listbox */
    GtkWidget *chat_msg_listbox = get_widget_by_name(gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), msg.chan), "chat_msg_listbox");
    assert(chat_msg_listbox);
    gtk_container_add(GTK_CONTAINER(chat_msg_listbox), sys_msg_label);

    g_object_unref(G_OBJECT(builder));
}
