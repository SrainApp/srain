#include <string.h>
#include <gtk/gtk.h>
#include <string.h>
#include <assert.h>
#include "ui.h"
#include "msg.h"

#define UI_BUILDER_GET_WIDGET(builder, widget) \
    widget = GTK_WIDGET(gtk_builder_get_object(builder, ""#widget"")); \
    assert(widget)

static GtkWidget *window;
static GtkWidget *chat_panel_stack;

static unsigned int nchat = 0;
static struct {
    Chat chat;
    GtkWidget *msg_listbox;
} chat_list[100]; // TODO replace with MACRO

static gint msg_label_popup_handler(GtkWidget *label, GdkEvent *event, GtkWidget *menu){
    if (event->button.button == 3 && !gtk_label_get_selection_bounds(GTK_LABEL(label), NULL, NULL)){
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button.button, event->button.time);
        return TRUE;
    }
    return FALSE;
}


static void ui_apply_css(GtkWidget *widget, GtkStyleProvider *provider){
    gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider, G_MAXUINT);

    if (GTK_IS_CONTAINER(widget))
        gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)ui_apply_css, provider);
}

void ui_window_init(){
    GtkBuilder *builder;
    GtkStyleProvider *provider;

    builder = gtk_builder_new_from_file( "../ui/window.glade");
    UI_BUILDER_GET_WIDGET(builder, window);
    UI_BUILDER_GET_WIDGET(builder, chat_panel_stack);

    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    /* load style */
    provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "../ui/spring_rain.css", NULL);
    ui_apply_css(window, provider);

    /* transition effect */
    gtk_stack_set_transition_type(GTK_STACK(chat_panel_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);

    /* display window */
    gtk_widget_show_all(window);

    g_object_unref(G_OBJECT(builder));
    g_object_unref(G_OBJECT(provider));
}

void ui_new_chat(const Chat chat){
    GtkBuilder *builder;
    GtkWidget *chat_name_label;
    GtkWidget *chat_topic_label;
    GtkWidget *chat_panel_box;
    GtkWidget *chat_msg_listbox;

    builder = gtk_builder_new_from_file( "../ui/chat_panel.glade");
    UI_BUILDER_GET_WIDGET(builder, chat_panel_box);
    UI_BUILDER_GET_WIDGET(builder, chat_msg_listbox);
    UI_BUILDER_GET_WIDGET(builder, chat_name_label);
    UI_BUILDER_GET_WIDGET(builder, chat_topic_label);

    gtk_label_set_text(GTK_LABEL(chat_name_label), chat.name);
    gtk_label_set_text(GTK_LABEL(chat_topic_label), chat.topic);
    gtk_stack_add_named(GTK_STACK(chat_panel_stack), chat_panel_box, (gchar *)chat.name);
    gtk_container_child_set(GTK_CONTAINER(chat_panel_stack), chat_panel_box, "title", (gchar *)chat.name, NULL);

    strncpy(chat_list[nchat].chat.name, chat.name, 20);
    // chat_list[nchat].msg_listbox = g_object_ref(chat_msg_listbox);
    chat_list[nchat].msg_listbox = chat_msg_listbox;
    nchat++;
    // TODO nchat will reach limit soon

    g_object_unref(G_OBJECT(builder));
}

void ui_rm_chat(const char *chat_name){
    GtkWidget *chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), chat_name);
    assert(chat_panel_box);
    gtk_container_remove(GTK_CONTAINER(chat_panel_stack), chat_panel_box);
}

void ui_send_msg(const MsgSend msg){
    int i;
    GtkBuilder *builder;
    GtkWidget *send_msg_bubble_box;
    GtkWidget *send_msg_label;
    GtkWidget *send_image;
    GtkWidget *send_time_label;
    GtkWidget *msg_bubble_menu;

    builder = gtk_builder_new_from_file( "../ui/msg_bubble.glade");
    if (msg.img) UI_BUILDER_GET_WIDGET(builder, send_image);
    UI_BUILDER_GET_WIDGET(builder, send_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, send_msg_label);
    UI_BUILDER_GET_WIDGET(builder, send_time_label);
    UI_BUILDER_GET_WIDGET(builder, msg_bubble_menu);

    gtk_label_set_text(GTK_LABEL(send_msg_label), msg.msg);
    gtk_label_set_text(GTK_LABEL(send_time_label), msg.time);

    g_signal_connect(G_OBJECT(send_msg_label), "event", G_CALLBACK(msg_label_popup_handler), G_OBJECT(msg_bubble_menu));
    g_object_ref(msg_bubble_menu); // TODO with out this statement, gtkmenu will be free after unref builder

    for (i = 0; i < nchat; i++){
        if (strcmp(msg.chan, chat_list[i].chat.name) == 0){
            gtk_container_add(GTK_CONTAINER(chat_list[i].msg_listbox), GTK_WIDGET(send_msg_bubble_box));
            break;
        }
    }
    g_object_unref(G_OBJECT(builder));
}

void ui_recv_msg(const MsgRecv msg){
    int i;
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

    g_signal_connect(G_OBJECT(recv_msg_label), "event", G_CALLBACK(msg_label_popup_handler), G_OBJECT(msg_bubble_menu));
    g_object_ref(msg_bubble_menu); // TODO with out this statement, gtkmenu will be free after unref builder

    for (i = 0; i < nchat; i++){
        if (strcmp(msg.chan, chat_list[i].chat.name) == 0){
            gtk_container_add(GTK_CONTAINER(chat_list[i].msg_listbox), GTK_WIDGET(recv_msg_bubble_box));
            break;
        }
    }
    g_object_unref(G_OBJECT(builder));
}

void ui_sys_msg(const MsgSys msg){
    int i;
    GtkBuilder *builder;
    GtkWidget *sys_msg_label;

    builder = gtk_builder_new_from_file( "../ui/msg_bubble.glade");
    UI_BUILDER_GET_WIDGET(builder, sys_msg_label);

    gtk_label_set_text(GTK_LABEL(sys_msg_label), msg.msg);

    for (i = 0; i < nchat; i++){
        if (strcmp(msg.chan, chat_list[i].chat.name) == 0){
            gtk_container_add(GTK_CONTAINER(chat_list[i].msg_listbox), GTK_WIDGET(sys_msg_label));
            break;
        }
    }
    g_object_unref(G_OBJECT(builder));
}

void ui_online_list_add(const char *nick){

}
void ui_online_list_remove(const char *nick){

}
void ui_online_list_init(const char **nick){

}
