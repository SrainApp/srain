#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "log.h"

void image_window_init(const gchar *path){
    GtkBuilder *builder;
    GtkWidget *image_window;
    GtkWidget *image;
    GdkScreen *screen;
    gint height, width;
    GdkPixbuf *pixbuf;

    builder = gtk_builder_new_from_file( "../data/ui/image_window.glade");
    UI_BUILDER_GET_WIDGET(builder, image_window);
    UI_BUILDER_GET_WIDGET(builder, image);

    g_signal_connect_swapped(image_window, "button_release_event", G_CALLBACK(gtk_widget_destroy), image_window);
    g_signal_connect_swapped(image, "button_release_event", G_CALLBACK(gtk_widget_destroy), image_window);

    screen = gdk_screen_get_default();
    height = gdk_screen_get_height(screen);
    width = gdk_screen_get_width(screen);
    // g_object_unref(screen);
    // Gdk-ERROR **: attempted to destroy root window

    LOG_FR("path = %s, height = %d, width = %d",path, height, width);

    pixbuf = gdk_pixbuf_new_from_file_at_size(path, width, height, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
    g_object_unref(pixbuf);

    gtk_widget_show_all(image_window);

    g_object_unref(builder);
}
