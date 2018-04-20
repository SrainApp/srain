/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file sui_url_previewer.c
 * @brief URL preview widget, supports HTTP for now,
 *      and may only support HTTP forever.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-04-01
 */

#include <gtk/gtk.h>
#include <libsoup/soup.h>
#include <string.h>

#include "sui_common.h"
#include "sui_url_previewer.h"

#include "log.h"
#include "utils.h"
#include "i18n.h"

#define STACK_PAGE_LOADING  "loading"
#define STACK_PAGE_TEXT     "text"
#define STACK_PAGE_IMAGE    "image"

#define THUMBNAIL_SIZE      300
#define MAX_CONTENT_LENGTH  10485760 // 10Mb
#define MAX_INSTANCE_COUNT  20

enum _SuiUrlContentType {
    SUI_URL_CONTENT_TYPE_UNSUPPORTED,
    SUI_URL_CONTENT_TYPE_TEXT,
    SUI_URL_CONTENT_TYPE_IMAGE,
    SUI_URL_CONTENT_TYPE_UNKNOWN,
};

struct _SuiUrlPreviewer {
    GtkBox parent;

    bool previewed;
    char *url;
    SoupURI *uri;
    SoupSession *session;
    SoupMessage *msg;
    GCancellable *cancel;

    GtkButton *open_button;
    GtkStack *stack;
    /* Page loading */
    GtkSpinner *spinner;
    GtkButton *cancel_button;
    /* Page text */
    GtkLabel *text_label;
    /* Page image */
    GtkEventBox *image_event_box;
    GdkPixbuf *pixbuf;
    GtkImage *image;
};

struct _SuiUrlPreviewerClass {
    GtkBoxClass parent_class;
};

static SoupSession *default_session = NULL;

static void sui_url_previewer_set_url(SuiUrlPreviewer *self, const char *url);
static const char* sui_url_previewer_get_url(SuiUrlPreviewer *self);

static void preview(SuiUrlPreviewer *self);
static void cancel_preview(SuiUrlPreviewer *self);
static void preview_text(SuiUrlPreviewer *self, const char *text);
static void preview_error_text(SuiUrlPreviewer *self,
        const char *text);
static void preview_image(SuiUrlPreviewer *self, GdkPixbuf *pixbuf);
static SuiUrlContentType get_url_content_type(SuiUrlPreviewer *self);

static void on_notify_visible(GObject *object, GParamSpec *pspec, gpointer data);
static void open_button_on_clicked(GtkWidget *widget, gpointer user_data);
static void cancel_button_on_clicked(GtkWidget *widget, gpointer user_data);
static void image_event_box_on_button_release(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data);
static void session_send_ready(GObject *object, GAsyncResult *result,
        gpointer user_data);
static void buffered_stream_fill_ready(GObject *object, GAsyncResult *result,
        gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_URL = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiUrlPreviewer, sui_url_previewer, GTK_TYPE_BOX);

static void sui_url_previewer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiUrlPreviewer *self;

  self = SUI_URL_PREVIEWER(object);

  switch (property_id){
    case PROP_URL:
      sui_url_previewer_set_url(self, g_value_get_string(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_url_previewer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiUrlPreviewer *self;

  self = SUI_URL_PREVIEWER(object);

  switch (property_id){
    case PROP_URL:
      g_value_set_string(value, sui_url_previewer_get_url(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_url_previewer_init(SuiUrlPreviewer *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    self->previewed = FALSE;
    self->cancel = g_cancellable_new();

    /* All SuiUrlPreviewers share one SoupSession instance */
    if (!SOUP_IS_SESSION(default_session)){
        default_session = soup_session_new_with_options(
                SOUP_SESSION_USER_AGENT, PACKAGE_NAME "/" PACKAGE_VERSION,
                SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
                NULL);
    }
    self->session = default_session;

    g_signal_connect(self, "notify::visible",
            G_CALLBACK(on_notify_visible), NULL);
    g_signal_connect(self->open_button, "clicked",
            G_CALLBACK(open_button_on_clicked), self);
    g_signal_connect(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_clicked), self);
    g_signal_connect(self->image_event_box, "button-release-event",
            G_CALLBACK(image_event_box_on_button_release), self);
}

static void sui_url_previewer_constructed(GObject *object){
    SuiUrlPreviewer *self;

    G_OBJECT_CLASS(sui_url_previewer_parent_class)->constructed(object);

    self = SUI_URL_PREVIEWER(object);
    gtk_widget_set_tooltip_text(GTK_WIDGET(self), self->url);
}

static void sui_url_previewer_finalize(GObject *object){
    SuiUrlPreviewer *self;

    self = SUI_URL_PREVIEWER(object);
    str_assign(&self->url, NULL);
    soup_uri_free(self->uri);
    g_object_unref(self->cancel);
    if (SOUP_IS_MESSAGE(self->msg)){
        g_object_unref(self->msg);
    }
    if (GDK_IS_PIXBUF(self->pixbuf)){
        g_object_unref(self->pixbuf);
    }

    G_OBJECT_CLASS(sui_url_previewer_parent_class)->finalize(object);
}

static void sui_url_previewer_class_init(SuiUrlPreviewerClass *class){
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS (class);
    object_class->set_property = sui_url_previewer_set_property;
    object_class->get_property = sui_url_previewer_get_property;
    object_class->constructed = sui_url_previewer_constructed;
    object_class->finalize = sui_url_previewer_finalize;

    /* Install properties */
    obj_properties[PROP_URL] =
        g_param_spec_string("url",
                "URL",
                "URL of URL previewer.",
                NULL, // Default value
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(object_class, N_PROPERTIES,
            obj_properties);

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/url_previewer.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, open_button);
    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, stack);
    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, spinner);
    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, cancel_button);
    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, text_label);
    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, image_event_box);
    gtk_widget_class_bind_template_child(widget_class, SuiUrlPreviewer, image);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiUrlPreviewer* sui_url_previewer_new_from_cache(const char *url){
    static GList *instance_list = NULL;
    GList *lst;
    SoupURI *uri;
    SuiUrlPreviewer *instance;

    while (g_list_length(instance_list) > MAX_INSTANCE_COUNT){
        SuiUrlPreviewer *cur;

        lst = g_list_last(instance_list);
        cur = SUI_URL_PREVIEWER(lst->data);
        g_object_unref(cur);
        instance_list = g_list_delete_link(instance_list, lst);
    }

    instance = NULL;
    uri = soup_uri_new(url);
    lst = instance_list;
    while (lst){
        SuiUrlPreviewer *cur;

        cur = SUI_URL_PREVIEWER(lst->data);
        if (soup_uri_equal(cur->uri, uri)){
            instance = cur;
            break;
        }
        lst = g_list_next(lst);
    }

    if (!instance){
        instance = sui_url_previewer_new(url);
        instance_list = g_list_append(instance_list, g_object_ref(instance));
    }

    soup_uri_free(uri);
    return instance;
}

SuiUrlPreviewer* sui_url_previewer_new(const char *url){
    return g_object_new(SUI_TYPE_URL_PREVIEWER,
            "url", url,
            NULL);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_url_previewer_set_url(SuiUrlPreviewer *self, const char *url){
    /* Only allow called for once */
    str_assign(&self->url, url);
    self->uri = soup_uri_new(url);
}

static const char* sui_url_previewer_get_url(SuiUrlPreviewer *self){
    return self->url;
}

static void preview(SuiUrlPreviewer *self){
    g_return_if_fail(!SOUP_IS_MESSAGE(self->msg));

    if (get_url_content_type(self) == SUI_URL_CONTENT_TYPE_UNSUPPORTED){
        preview_error_text(self, _("Unsupported URL content type"));
        return;
    }
    if (self->previewed){
        return;
    }

    gtk_stack_set_visible_child_name(self->stack, STACK_PAGE_LOADING);

    g_cancellable_reset(self->cancel);
    self->msg = soup_message_new_from_uri("GET", self->uri);
    soup_session_send_async(self->session, self->msg, self->cancel,
            session_send_ready, self);
}

static void cancel_preview(SuiUrlPreviewer *self){
    if (!self->previewed){
        return;
    }
    g_cancellable_cancel(self->cancel);
}

static void preview_text(SuiUrlPreviewer *self, const char *text){
    self->previewed = TRUE;
    gtk_stack_set_visible_child_name(self->stack, STACK_PAGE_TEXT);
    gtk_label_set_text(self->text_label, text);
}

static void preview_error_text(SuiUrlPreviewer *self,
        const char *text){
    // preview_error_text means the preview is not completed
    self->previewed = FALSE;
    gtk_stack_set_visible_child_name(self->stack, STACK_PAGE_TEXT);
    gtk_label_set_text(self->text_label, text);
}

static void preview_image(SuiUrlPreviewer *self, GdkPixbuf *pixbuf){
    int width;
    int height;
    GdkPixbuf *scaled_pixbuf;

    self->previewed = TRUE;
    gtk_stack_set_visible_child_name(self->stack, STACK_PAGE_IMAGE);

    scale_size_to(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf),
            THUMBNAIL_SIZE, THUMBNAIL_SIZE, &width, &height);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height,
                        GDK_INTERP_BILINEAR);

    self->pixbuf = g_object_ref(pixbuf);
    gtk_image_set_from_pixbuf(self->image, scaled_pixbuf);
    g_object_unref(scaled_pixbuf);
}

static SuiUrlContentType get_url_content_type(SuiUrlPreviewer *self){
    const char *scheme;
    const char *content_type;
    SoupMessageHeaders *headers;

    scheme = soup_uri_get_scheme(self->uri);
    if (g_ascii_strcasecmp(scheme, "http") != 0
            && g_ascii_strcasecmp(scheme, "https") != 0){
        // Unsupported protocol
        return SUI_URL_CONTENT_TYPE_UNSUPPORTED;
    }

    if (!SOUP_IS_MESSAGE(self->msg)){
        return SUI_URL_CONTENT_TYPE_UNKNOWN;
    }

    headers = self->msg->response_headers;
    content_type = soup_message_headers_get_content_type(headers, NULL);
    if (g_str_has_prefix(content_type, "image/")){
        return SUI_URL_CONTENT_TYPE_IMAGE;
    }

    return SUI_URL_CONTENT_TYPE_UNSUPPORTED;
}

static void on_notify_visible(GObject *object, GParamSpec *pspec, gpointer data){
    SuiUrlPreviewer *self;

    self = SUI_URL_PREVIEWER(object);
    if (!gtk_widget_is_visible(GTK_WIDGET(self))){
        return;
    }
    preview(self);
}

static void open_button_on_clicked(GtkWidget *widget, gpointer user_data){
    SuiUrlPreviewer *self;

    self = SUI_URL_PREVIEWER(user_data);
    sui_open_url(self->url);
    sui_popdown_popover(GTK_WIDGET(self));
}

static void cancel_button_on_clicked(GtkWidget *widget, gpointer user_data){
    SuiUrlPreviewer *self;

    self = SUI_URL_PREVIEWER(user_data);
    cancel_preview(self);
    sui_popdown_popover(GTK_WIDGET(self));
}

static void image_event_box_on_button_release(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    int width;
    int height;
#if GTK_CHECK_VERSION(3, 22, 0)
    GdkDisplay *display;
    GdkMonitor *monitor;
#else
    GdkScreen *screen;
    int monitor;
#endif
    GdkWindow *gdkwin;
    GdkRectangle rect;
    GdkPixbuf *pixbuf;
    GdkPixbuf *scaled_pixbuf;
    GtkImage *image;
    GtkWindow *iwin;
    GtkBuilder *builder;
    SuiUrlPreviewer *self;

    if (event->button != 1){ // Left mouse button
        return;
    }

    self = SUI_URL_PREVIEWER(user_data);
    builder = gtk_builder_new_from_resource("/im/srain/Srain/image_window.glade");
    iwin = GTK_WINDOW(gtk_builder_get_object(builder, "image_window"));
    image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));

#if GTK_CHECK_VERSION(3, 22, 0)
    display = gdk_display_get_default();
    gdkwin = gtk_widget_get_window(GTK_WIDGET(sui_get_cur_window()));
    monitor = gdk_display_get_monitor_at_window(display, gdkwin);
    gdk_monitor_get_geometry(monitor, &rect);
#else
    screen = gdk_screen_get_default();
    gdkwin = gtk_widget_get_window(GTK_WIDGET(sui_get_cur_window()));
    monitor = gdk_screen_get_monitor_at_window(screen, gdkwin);
    gdk_screen_get_monitor_geometry(screen, monitor, &rect);
#endif

    /* If we should scale the image, do not fill full screen */
    rect.height -= 20;
    rect.width -= 20;
    pixbuf = self->pixbuf;
    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    scale_size_to(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf),
            rect.width, rect.height, &width, &height);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height,
            GDK_INTERP_BILINEAR);

    gtk_image_set_from_pixbuf(image, scaled_pixbuf);

    g_signal_connect_swapped(iwin, "button-release-event",
            G_CALLBACK(gtk_widget_destroy), iwin);
    g_signal_connect_swapped(image, "button-release-event",
            G_CALLBACK(gtk_widget_destroy), iwin);

    g_object_unref(scaled_pixbuf);
    g_object_unref(builder);

    gtk_window_present(iwin);
}

static void session_send_ready(GObject *object, GAsyncResult *result,
        gpointer user_data){
    int len;
	GError *err;
    GInputStream *input_stream;
    GBufferedInputStream *buffered_stream;
    SoupSession *session;
    SoupMessageHeaders *headers;
    SuiUrlPreviewer *self;

    session = SOUP_SESSION(object);
    self = SUI_URL_PREVIEWER(user_data);

    err = NULL;
	input_stream = soup_session_send_finish(session, result, &err);
    if (err) {
        preview_error_text(self, err->message);
        goto ERR;
    }

    headers = self->msg->response_headers;
    // TODO: chunked encoding support
    len = soup_message_headers_get_content_length(headers);
    if (len > MAX_CONTENT_LENGTH) {
        preview_error_text(self, _("Exceed max content length"));
        goto ERR;
    }

    switch (get_url_content_type(self)){
        case SUI_URL_CONTENT_TYPE_UNKNOWN:
            g_warn_if_reached();
            goto ERR;
            break;
        case SUI_URL_CONTENT_TYPE_UNSUPPORTED:
            preview_error_text(self, _("Unsupported content type"));
            goto ERR;
            break;
        default:
            buffered_stream = G_BUFFERED_INPUT_STREAM(
                    g_buffered_input_stream_new(input_stream));
            g_object_unref(input_stream);
            g_buffered_input_stream_set_buffer_size(buffered_stream, len); // 10Mb
            g_buffered_input_stream_fill_async(buffered_stream, -1,
                    G_PRIORITY_LOW, self->cancel, buffered_stream_fill_ready, self);
    }

    return;
ERR:
    if (G_IS_INPUT_STREAM(input_stream)){
        g_object_unref(input_stream);
    }
    g_object_unref(self->msg);
    self->msg = NULL;
}

static void buffered_stream_fill_ready(GObject *object, GAsyncResult *result,
        gpointer user_data){
    gsize len;
    const guchar *buf;
    GError *err;
    GBufferedInputStream *buffered_stream;
    SuiUrlPreviewer *self;

    buffered_stream = G_BUFFERED_INPUT_STREAM(object);
    self = SUI_URL_PREVIEWER(user_data);

    err = NULL;
    g_buffered_input_stream_fill_finish(buffered_stream, result, &err);
    if (err) {
        preview_error_text(self, err->message);
        goto FIN;
    }

    {
        int avail;
        int size;

        avail = g_buffered_input_stream_get_available(buffered_stream);
        size = g_buffered_input_stream_get_buffer_size(buffered_stream);
        if (avail != size){
            g_buffered_input_stream_fill_async(buffered_stream, -1,
                    G_PRIORITY_LOW, self->cancel, buffered_stream_fill_ready, self);
            return;
        }
    }

    buf = g_buffered_input_stream_peek_buffer(buffered_stream, &len);

    switch (get_url_content_type(self)){
        case SUI_URL_CONTENT_TYPE_IMAGE:
            {
                GdkPixbuf *pixbuf;
                GdkPixbufLoader *loader;

                loader = gdk_pixbuf_loader_new();
                err = NULL;
                gdk_pixbuf_loader_write(loader, buf, len, &err);
                if (err) {
                    WARN_FR("Failed to write to GdkPixbufLoader: %s", err->message);
                    gdk_pixbuf_loader_close(loader, NULL);
                    g_object_unref(loader);
                    preview_error_text(self, err->message);
                    goto FIN;
                }
                pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
                if (pixbuf){
                    preview_image(self, pixbuf);
                } else {
                    WARN_FR("Get a NULL pixbuf from pixbuf loader");
                }
                gdk_pixbuf_loader_close(loader, NULL);
                g_object_unref(loader);
                break;
            }
        default:
            g_warn_if_reached();
    }

FIN:
    g_object_unref(buffered_stream);
    g_object_unref(self->msg);
    self->msg = NULL;
}
