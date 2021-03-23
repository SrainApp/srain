/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
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

#include <gtk/gtk.h>

#include "srn-meta.h"
#include "srn-window.h"

#define WINDOW_STACK_PAGE_WELCOME   "welcome"
#define WINDOW_STACK_PAGE_MAIN      "main"

struct _SrnWindow {
    GtkApplicationWindow parent;

    /* Top level container */
    GtkPaned *title_paned;
    GtkBox *window_box;
    GtkSeparator *header_separator;
    GtkBox *header_box;
    GtkPaned *header_paned;
    GtkStack *window_stack;

    /* Side header */
    GtkHeaderBar *side_header_bar;
    GtkBox *side_header_box;
    GtkBox *side_left_header_box;
    GtkBox *side_right_header_box;
    GtkImage *start_image;
    GtkMenuButton *start_menu_button;

    /* Buffer header */
    GtkHeaderBar *buffer_header_bar;
    GtkBox *buffer_header_box;
    GtkBox *buffer_title_box;
    GtkLabel *buffer_title_label;
    GtkLabel *buffer_subtitle_label;
    GtkMenuButton *buffer_menu_button;

    /* Welcome page */
    GtkBox *welcome_connect_box;

    /* Main page */
    GtkPaned *main_paned;
    GtkBox *side_box;
    GtkStack *buffer_stack;
    GtkButton *plugin_button;
    GtkTextView *input_text_view;
    GtkButton *send_button;
};

enum {
    PROP_0,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SrnWindow, srn_window, GTK_TYPE_APPLICATION_WINDOW);

static void
on_destroy(SrnWindow *self) {
    // Nothing to do for now
}

static void
on_notify_is_active(GObject *object, GParamSpec *pspec,
                    gpointer data) {
}

static void
window_stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
                              gpointer user_data) {
}

static void
buffer_stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
                              gpointer user_data) {
}

static void
srn_window_set_property(GObject *object, guint property_id,
                        const GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_window_get_property(GObject *object, guint property_id,
                        GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_window_init(SrnWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    /* Bind title_paned, header_paned and main_paned */
    g_object_bind_property(
        self->title_paned, "position",
        self->main_paned, "position",
        G_BINDING_BIDIRECTIONAL);
    g_object_bind_property(
        self->header_paned, "position",
        self->main_paned, "position",
        G_BINDING_BIDIRECTIONAL);

    g_signal_connect(self, "destroy",
                     G_CALLBACK(on_destroy), NULL);
    g_signal_connect(self, "notify::is-active",
                     G_CALLBACK(on_notify_is_active), NULL);

    g_signal_connect(self->window_stack, "notify::visible-child",
                     G_CALLBACK(window_stack_on_child_changed), self);
    g_signal_connect(self->buffer_stack, "notify::visible-child",
                     G_CALLBACK(buffer_stack_on_child_changed), self);
}

static void
srn_window_constructed(GObject *object) {
    SrnWindow *self;

    self = SRN_WINDOW(object);
    G_OBJECT_CLASS(srn_window_parent_class)->constructed(object);
}

static void
srn_window_class_init(SrnWindowClass *class) {
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_window_constructed;
    object_class->set_property = srn_window_set_property;
    object_class->get_property = srn_window_get_property;

    /* Install properties */
    g_object_class_install_properties(object_class, N_PROPERTIES,
                                      obj_properties);

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/window.glade");

    gtk_widget_class_bind_template_child(widget_class, SrnWindow, title_paned);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, window_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, header_separator);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, header_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, header_paned);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, window_stack);

    gtk_widget_class_bind_template_child(widget_class, SrnWindow, side_header_bar);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, side_header_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         side_left_header_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         side_right_header_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, start_image);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         start_menu_button);

    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         buffer_header_bar);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         buffer_header_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, buffer_title_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         buffer_title_label);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         buffer_subtitle_label);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         buffer_menu_button);

    gtk_widget_class_bind_template_child(widget_class, SrnWindow,
                                         welcome_connect_box);

    gtk_widget_class_bind_template_child(widget_class, SrnWindow, main_paned);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, side_box);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, buffer_stack);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, plugin_button);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, input_text_view);
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, send_button);
}

SrnWindow *
srn_window_new(SrnApplication *app) {
    return g_object_new(SRN_TYPE_WINDOW,
                        "application", GTK_APPLICATION(app),
                        NULL);
}
