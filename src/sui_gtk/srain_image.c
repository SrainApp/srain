/**
 * @file srain_image.c
 * @brief a widget which can load image from url asynchronously
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-04-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "sui_common.h"
#include "srain_image.h"

#include "download.h"
#include "log.h"

struct _SrainImage {
    GtkBox parent;

    int size;
    char *url;
    char *file;
    SrainImageType type;

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

static void srain_image_finalize(GObject *object){
    if (SRAIN_IMAGE(object)->file)
        free(SRAIN_IMAGE(object)->file);

    G_OBJECT_CLASS(srain_image_parent_class)->finalize(object);
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
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    object_class->finalize = srain_image_finalize;
}

SrainImage* srain_image_new(void){
    return g_object_new(SRAIN_TYPE_IMAGE, NULL);
}

/**
 * @brief eventbox_on_click
 *
 * @param user_data: a instance of SrainImage
 * @param event
 *
 * New a popup window to display image at its origin size,
 * if image is too large, it will be scaled.
 */
static void eventbox_on_click(gpointer user_data , GdkEventButton *event){
    int monitor;
    int width;
    int height;
    GdkScreen *screen;
    GdkWindow *gdkwin;
    GdkRectangle rect;
    GdkPixbuf *pixbuf;
    GdkPixbuf *pixbuf2;
    GtkImage *image;
    GtkWindow *iwin;
    GtkBuilder *builder;
    SrainImage *simg;

    if (event->button == 1){
        simg = user_data;
        if (simg->file == NULL) return;

        builder = gtk_builder_new_from_resource("/org/gtk/srain/image_window.glade");
        iwin = GTK_WINDOW(gtk_builder_get_object(builder, "image_window"));

        screen = gdk_screen_get_default();
        gdkwin = gtk_widget_get_window((gtk_widget_get_toplevel(GTK_WIDGET(simg))));
        monitor = gdk_screen_get_monitor_at_window(screen, gdkwin);
        gdk_screen_get_monitor_geometry(screen, monitor, &rect);

        /* If we should scale the image, do not fill full screen */
        rect.height -= 20;
        rect.width -= 20;

        image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));
        pixbuf = gdk_pixbuf_new_from_file(simg->file, NULL);

        width = gdk_pixbuf_get_width(pixbuf);
        height = gdk_pixbuf_get_height(pixbuf);

        scale_size_to(width, height, rect.width, rect.height, &width, &height);

        pixbuf2 = gdk_pixbuf_scale_simple(pixbuf, width, height,
                        GDK_INTERP_BILINEAR);

        gtk_image_set_from_pixbuf(image, pixbuf2);

        g_signal_connect_swapped(iwin, "button-release-event",
                G_CALLBACK(gtk_widget_destroy), iwin);
        g_signal_connect_swapped(image, "button-release-event",
                G_CALLBACK(gtk_widget_destroy), iwin);

        g_object_unref(pixbuf2);
        g_object_unref(pixbuf);
        g_object_unref(builder);

        gtk_window_present(iwin);
    }
}

static void srain_image_set_from_self(SrainImage *simg){

    if (simg->size == 0){
        gtk_image_set_from_file(simg->image, simg->file);
    } else {
        GError *error;
        GdkPixbuf *pixbuf;
        GdkPixbuf *pixbuf2;
        int width, height;

        error = NULL;
        pixbuf = gdk_pixbuf_new_from_file(simg->file, &error);

        width = gdk_pixbuf_get_width(pixbuf);
        height = gdk_pixbuf_get_height(pixbuf);

        scale_size_to(width, height, simg->size, simg->size, &width, &height);

        pixbuf2 = gdk_pixbuf_scale_simple(pixbuf, width, height,
                        GDK_INTERP_BILINEAR);

        if (error){
            ERR_FR("failed to open %s as a image", simg->file);
            return;
        }

        gtk_widget_show(GTK_WIDGET(simg->image));
        gtk_image_set_from_pixbuf(simg->image, pixbuf2);

        g_object_unref(pixbuf2);
        g_object_unref(pixbuf);
    }

    if (simg->type & SRAIN_IMAGE_ENLARGE){
        g_signal_connect_swapped(simg, "button_release_event",
                G_CALLBACK(eventbox_on_click), simg);
    }
}

static gboolean set_image_idle(SrainImage *simg){

    /* Check whether object alve now */
    g_return_val_if_fail(SRAIN_IS_IMAGE(simg), FALSE);

    free(simg->url);
    simg->url = NULL;

    gtk_spinner_stop(simg->spinner);
    gtk_widget_set_visible(GTK_WIDGET(simg->spinner), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(simg->image), TRUE);

    srain_image_set_from_self(simg);

    return FALSE;
}

static void download_image(SrainImage *simg){
    GString *str;

    str = download(simg->url);

    if (str){
        if (simg->file) free(simg->file);
        simg->file = strdup(str->str);
        g_string_free(str, TRUE);

        gdk_threads_add_idle((GSourceFunc)set_image_idle, simg);
    }
}

static void set_image_from_url_async(SrainImage *simg){
    if (simg->type & SRAIN_IMAGE_SPININER){
        gtk_widget_set_visible(GTK_WIDGET(simg->spinner), TRUE);
        gtk_spinner_start(simg->spinner);
    }

    g_thread_new(NULL, (GThreadFunc)download_image, simg);

    if (!(simg->type & SRAIN_IMAGE_AUTOLOAD)){
        gtk_widget_set_visible(GTK_WIDGET(simg->load_button), FALSE);
    }
}

/**
 * @brief srain_image_set_from_file
 *
 * @param simg a SrainImage instance
 * @param file path of file
 * @param size image size (both width and height)
 * @param type see SrainImageType in inc/srain_image.h
 *      SRAIN_IMAGE_AUTOLOAD and SRAIN_IMAGE_SPININER are
 *      not used by this func
 */
void srain_image_set_from_file(SrainImage *simg, char *file,
        int size, SrainImageType type){
    simg->size = size;
    simg->type = type;

    if (simg->file) free(simg->file);
    simg->file = strdup(file);

    srain_image_set_from_self(simg);
}

/**
 * @brief srain_image_set_from_url_async
 *
 * @param simg a SrainImage instance
 * @param file path of file
 * @param size image size (both width and height)
 * @param type see SrainImageType in inc/srain_image.h
 */
void srain_image_set_from_url_async(SrainImage *simg, const char *url,
        int size, SrainImageType type){
    simg->size = size;
    simg->type = type;

    if (simg->url) free(simg->url);
    simg->url = strdup(url);

    if (type & SRAIN_IMAGE_AUTOLOAD){
        set_image_from_url_async(simg);
    } else {
        gtk_widget_set_visible(GTK_WIDGET(simg->load_button), TRUE);
        g_signal_connect_swapped(simg->load_button, "clicked",
                G_CALLBACK(set_image_from_url_async), simg);
    }
}
