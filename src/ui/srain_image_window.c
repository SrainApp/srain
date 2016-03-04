/**
 * @file srain_image_window.c
 * @brief a popup windows used to display image in message
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "theme.h"
#include "srain_image_window.h"
#include "log.h"

struct _SrainImageWindow {
    GtkWindow parent;
    GtkImage *image;
};

struct _SrainImageWindowClass {
    GtkWindowClass parent_class;
};

G_DEFINE_TYPE(SrainImageWindow, srain_image_window, GTK_TYPE_WINDOW);

static void srain_image_window_init(SrainImageWindow *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_image_window_class_init(SrainImageWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/image_window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainImageWindow, image);
}

SrainImageWindow* srain_image_window_new(const char *path){
    gint height, width;
    GdkScreen *screen;
    GdkPixbuf *pixbuf;
    SrainImageWindow *win;

    win = g_object_new(SRAIN_TYPE_IMAGE_WINDOW, NULL);

    screen = gdk_screen_get_default();
    height = gdk_screen_get_height(screen);
    width = gdk_screen_get_width(screen);
    pixbuf = gdk_pixbuf_new_from_file_at_size(path, width - 20, height - 20, NULL);
    gtk_image_set_from_pixbuf(win->image, pixbuf);
    gtk_widget_show_all(GTK_WIDGET(win));

    g_signal_connect_swapped(win, "button_release_event", G_CALLBACK(gtk_widget_destroy), win);
    g_signal_connect_swapped(win->image, "button_release_event", G_CALLBACK(gtk_widget_destroy), win);

    g_object_unref(pixbuf);

    theme_apply(GTK_WIDGET(win));
    // TODO 为什么现在的 image window 可以 resize 了？ 这不是理想中的效果
    return win;
}
