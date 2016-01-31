#include <string.h>
#include <gtk/gtk.h>
#include "ui.h"

static GtkWidget *window;
static GtkWidget *chat_list_sidebar;
static GtkWidget *chat_panel_stack;
static GtkWidget *chat_msg_listbox;
static unsigned int nchan = 0;
static Chan chan_list[100];

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
    window = GTK_WIDGET(gtk_builder_get_object(builder, "srain_window"));
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    // gtk_builder_connect_signals(builder, NULL);
    chat_panel_stack = GTK_WIDGET(gtk_builder_get_object(builder, "chat_panel_stack"));
    chat_list_sidebar = GTK_WIDGET(gtk_builder_get_object(builder, "chat_list_sidebar"));
    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(chat_list_sidebar), GTK_STACK(chat_panel_stack));

    g_object_unref(G_OBJECT(builder));

    /* load style */
    provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "../ui/spring_rain.css", NULL);
    ui_apply_css(window, provider);

    /* transition effect */
    gtk_stack_set_transition_type(GTK_STACK(chat_panel_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);

    /* display window */
    gtk_widget_show_all(window);
}

void ui_join_chan(const char *chan){
    GtkBuilder *builder;
    GtkWidget *chat_panel_box;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/chat_panel.glade", NULL);
    chat_panel_box = GTK_WIDGET(gtk_builder_get_object(builder, "chat_panel_box"));
    chat_msg_listbox = GTK_WIDGET(gtk_builder_get_object(builder, "chat_msg_listbox"));

    gtk_stack_add_named(GTK_STACK(chat_panel_stack), chat_panel_box, (gchar *)chan);
    gtk_container_child_set(GTK_CONTAINER(chat_panel_stack), chat_panel_box, "title", (gchar *)chan, NULL);

    strncpy(chan_list[nchan].name, chan, 20);
    chan_list[nchan].panel = chat_msg_listbox;
    nchan++;

    g_object_unref(G_OBJECT(builder));
}

void ui_part_chan(Chan chan){

}

void ui_send_msg(const char *msg){
    GtkBuilder *builder;
    GtkWidget *send_msg_bubble;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/msg_bubble.glade", NULL);
    send_msg_bubble = GTK_WIDGET(gtk_builder_get_object(builder, "send_msg_bubble_box"));
    // gtk_widget_show(recv_msg_bubble);
    gtk_container_add(GTK_CONTAINER(chan_list[0].panel), GTK_WIDGET(send_msg_bubble));
}

void ui_recv_msg(const char *msg){
    GtkBuilder *builder;
    GtkWidget *recv_msg_bubble;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/msg_bubble.glade", NULL);
    recv_msg_bubble = GTK_WIDGET(gtk_builder_get_object(builder, "recv_msg_bubble_box"));
    // gtk_widget_show(recv_msg_bubble);
    gtk_container_add(GTK_CONTAINER(chan_list[0].panel), GTK_WIDGET(recv_msg_bubble));
}

void ui_sys_msg(const char *msg){
    GtkBuilder *builder;
    GtkWidget *recv_msg_bubble;
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/msg_bubble.glade", NULL);
    recv_msg_bubble = GTK_WIDGET(gtk_builder_get_object(builder, "sys_msg_label"));
    // gtk_widget_show(recv_msg_bubble);
    gtk_container_add(GTK_CONTAINER(chan_list[0].panel), GTK_WIDGET(recv_msg_bubble));

}
