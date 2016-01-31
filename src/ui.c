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

static unsigned int nchan = 0;
static Chan chan_list[100]; // TODO replace with MACRO

static void ui_apply_css(GtkWidget *widget, GtkStyleProvider *provider){
    gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider, G_MAXUINT);

    if (GTK_IS_CONTAINER(widget))
        gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)ui_apply_css, provider);
}

void ui_window_init(){
    GtkBuilder *builder;
    GtkStyleProvider *provider;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/window.glade", NULL);
    UI_BUILDER_GET_WIDGET(builder, window);
    UI_BUILDER_GET_WIDGET(builder, chat_panel_stack);

    // gtk_builder_connect_signals(builder, NULL);
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

void ui_join_chan(const char *chan){
    GtkBuilder *builder;
    GtkWidget *chat_panel_box;
    GtkWidget *chat_msg_listbox;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/chat_panel.glade", NULL);
    UI_BUILDER_GET_WIDGET(builder, chat_panel_box);
    UI_BUILDER_GET_WIDGET(builder, chat_msg_listbox);

    gtk_stack_add_named(GTK_STACK(chat_panel_stack), chat_panel_box, (gchar *)chan);
    gtk_container_child_set(GTK_CONTAINER(chat_panel_stack), chat_panel_box, "title", (gchar *)chan, NULL);

    strncpy(chan_list[nchan].name, chan, 20);
    chan_list[nchan].panel = chat_msg_listbox;
    printf("%s: %d\n", chan, nchan);
    nchan++;

    g_object_unref(G_OBJECT(builder));
}

void ui_part_chan(Chan chan){

}

void ui_send_msg(const MsgSend msg){
    int i;
    GtkBuilder *builder;
    GtkWidget *send_msg_bubble_box;
    GtkWidget *send_msg_label;
    GtkWidget *send_image;
    GtkWidget *send_time_label;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/msg_bubble.glade", NULL);
    if (msg.img) UI_BUILDER_GET_WIDGET(builder, send_image);
    UI_BUILDER_GET_WIDGET(builder, send_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, send_msg_label);
    UI_BUILDER_GET_WIDGET(builder, send_time_label);

    gtk_label_set_text(GTK_LABEL(send_msg_label), msg.msg);
    gtk_label_set_text(GTK_LABEL(send_time_label), msg.time);

    for (i = 0; i < nchan; i++){
        if (strcmp(msg.chan, chan_list[i].name) == 0){
            gtk_container_add(GTK_CONTAINER(chan_list[i].panel), GTK_WIDGET(send_msg_bubble_box));
            return;
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

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/msg_bubble.glade", NULL);
    if (msg.avatar) UI_BUILDER_GET_WIDGET(builder, avatar_image);
    if (msg.img) UI_BUILDER_GET_WIDGET(builder, recv_image);
    UI_BUILDER_GET_WIDGET(builder, recv_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, nick_label);
    UI_BUILDER_GET_WIDGET(builder, nick_button);
    UI_BUILDER_GET_WIDGET(builder, identify_label);
    UI_BUILDER_GET_WIDGET(builder, recv_msg_label);
    UI_BUILDER_GET_WIDGET(builder, recv_time_label);

    gtk_label_set_text(GTK_LABEL(nick_label), msg.nick);
    gtk_label_set_text(GTK_LABEL(identify_label), msg.id);
    gtk_label_set_text(GTK_LABEL(recv_time_label), msg.time);
    gtk_label_set_text(GTK_LABEL(recv_msg_label), msg.msg);

    for (i = 0; i < nchan; i++){
        printf("%s and %s %d\n", msg.chan, chan_list[i].name, i);
        if (strcmp(msg.chan, chan_list[i].name) == 0){
            gtk_container_add(GTK_CONTAINER(chan_list[i].panel), GTK_WIDGET(recv_msg_bubble_box));
            return;
        }
    }
    g_object_unref(G_OBJECT(builder));
}

void ui_sys_msg(const MsgSys msg){
    int i;
    GtkBuilder *builder;
    GtkWidget *sys_msg_label;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/msg_bubble.glade", NULL);
    UI_BUILDER_GET_WIDGET(builder, sys_msg_label);

    gtk_label_set_text(GTK_LABEL(sys_msg_label), msg.msg);

    for (i = 0; i < nchan; i++){
        if (strcmp(msg.chan, chan_list[i].name) == 0){
            gtk_container_add(GTK_CONTAINER(chan_list[i].panel), GTK_WIDGET(sys_msg_label));
            return;
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
