#include <stdio.h>
#include <gtk/gtk.h>
#include "i18n.h"

static void print_hello (GtkWidget *widget, gpointer data){
    g_print(_("Hello srain\n"));
}

static void activate(GtkApplication* app, gpointer user_data){
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *button_box;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW (window), "Srain");
    gtk_window_set_default_size(GTK_WINDOW (window), 200, 200);

    button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(window), button_box);
    button = gtk_button_new_with_label(_("Hello srain"));
    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    gtk_container_add(GTK_CONTAINER(button_box), button);

    gtk_widget_show_all(window);
}


int main(int argc, char **argv){
    int status;
    GtkApplication *app;

    setlocale(LC_ALL, "");
    textdomain("srain");
    bindtextdomain("srain", "locale");
    printf(_("Hello srain!\n"));


    app = gtk_application_new("org.gtk.srain", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run (G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
