/**
 * @file srain_image.c
 * @brief a widget that can load image from url asynchronously
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include "srain_image.h"
#include "download.h"
#include "log.h"

struct _SrainImage {
    GtkBox parent;
    const char *url;
    GString *filename;
    GtkButton *load_button;
    GtkSpinner *spinner;
    GtkImage *image;
    GtkEventBox *eventbox;
    GtkWindow *window;
};

struct _SrainImageClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainImage, srain_image, GTK_TYPE_BOX);

static void eventbox_on_click(gpointer user_data , GdkEventButton *event){
    char *path;
    gint height, width;
    GdkScreen *screen;
    GdkPixbuf *pixbuf;
    GtkImage *image;
    GtkWindow *win;
    GtkBuilder *builder;

    if (event->button == 1){
        path = user_data;

        builder = gtk_builder_new_from_resource("/org/gtk/srain/image_window.glade");
        win = GTK_WINDOW(gtk_builder_get_object(builder, "image_window"));
        image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));

        screen = gdk_screen_get_default();
        height = gdk_screen_get_height(screen);
        width = gdk_screen_get_width(screen);
        pixbuf = gdk_pixbuf_new_from_file_at_size(path, width - 20, height - 20, NULL);
        gtk_image_set_from_pixbuf(image, pixbuf);

        g_signal_connect_swapped(win, "button_release_event",
                G_CALLBACK(gtk_widget_destroy), win);
        g_signal_connect_swapped(image, "button_release_event",
                G_CALLBACK(gtk_widget_destroy), win);

        g_object_unref(builder);
        g_object_unref(pixbuf);

        // theme_apply(GTK_WIDGET(win));
        gtk_window_present(win);
    }
}

static gboolean set_image_idle(SrainImage *simg){
    GError *error;
    GdkPixbuf *pixbuf;

    LOG_FR("filename: '%s'", simg->filename->str);

    error = NULL;
    simg->url = NULL;

    gtk_spinner_stop(simg->spinner);
    gtk_widget_set_visible(GTK_WIDGET(simg->spinner), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(simg->image), TRUE);

    pixbuf = gdk_pixbuf_new_from_file_at_size(simg->filename->str,
            300, 300, &error);

    if (error){ 
        ERR_FR("failed to open %s as a image", simg->filename->str);
        return FALSE;
    }

    gtk_image_set_from_pixbuf(simg->image, pixbuf);

    g_signal_connect_swapped(simg->eventbox, "button_release_event",
            G_CALLBACK(eventbox_on_click), simg->filename->str);

    g_object_unref(pixbuf);

    return FALSE;
}

static void download_image_async(SrainImage *simg){
    // TODO: free simg->filename when destroy
    LOG_FR("url: '%s'", simg->url);

    simg->filename = download(simg->url);

    if (simg->filename){
        gdk_threads_add_idle((GSourceFunc)set_image_idle, simg);
    }
}

static void load_image(SrainImage *simg){
    gtk_widget_set_visible(GTK_WIDGET(simg->spinner), TRUE);
    gtk_spinner_start(simg->spinner);
    g_thread_new(NULL, (GThreadFunc)download_image_async, simg);

    gtk_widget_set_visible(GTK_WIDGET(simg->load_button), FALSE);
}

static void srain_image_init(SrainImage*self){
    gtk_widget_init_template(GTK_WIDGET(self));

    gtk_widget_set_visible(GTK_WIDGET(self->spinner), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(self->image), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(self->load_button), FALSE);
}

static void srain_image_class_init(SrainImageClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/image.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainImage, load_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainImage, spinner);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainImage, image);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainImage, eventbox);
}

SrainImage* srain_image_new(void){
    return g_object_new(SRAIN_TYPE_IMAGE, NULL);
}

SrainImage* srain_image_new_from_url_async(const char *url, int auto_load){
    SrainImage *simg;

    simg = srain_image_new();
    simg->url = url;

    if (auto_load){
        load_image(simg);
    } else {
        gtk_widget_set_visible(GTK_WIDGET(simg->load_button), TRUE);
        g_signal_connect_swapped(simg->load_button, "clicked",
                G_CALLBACK(load_image), simg);
    }

    return simg;
}
