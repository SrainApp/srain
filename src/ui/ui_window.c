#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"

GtkWidget *chat_panel_stack;

static void apply_css(GtkWidget *widget, GtkStyleProvider *provider){
    gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider, G_MAXUINT);

    if (GTK_IS_CONTAINER(widget))
        gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)apply_css, provider);
}

void ui_window_init(){
    GtkBuilder *builder;
    GtkStyleProvider *provider;
    GtkWidget *window;

    builder = gtk_builder_new_from_file( "../data/ui/window.glade");
    UI_BUILDER_GET_WIDGET(builder, window);
    UI_BUILDER_GET_WIDGET(builder, chat_panel_stack);

    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    /* load style */
    provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "../ui/spring_rain.css", NULL);
    apply_css(window, provider);

    /* transition effect */
    gtk_stack_set_transition_type(GTK_STACK(chat_panel_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);

    /* display window */
    gtk_widget_show_all(window);

    g_object_unref(G_OBJECT(builder));
    g_object_unref(G_OBJECT(provider));
}
