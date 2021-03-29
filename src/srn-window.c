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
#include "srn-flow-controller.h"

/**
 * SrnWindow:
 *
 * Srain's main window widget.
 */

struct _SrnWindow {
    GtkApplicationWindow parent;

    GtkCenterBox *center_box;
};

enum {
    PROP_0,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SrnWindow, srn_window, GTK_TYPE_APPLICATION_WINDOW);

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


    gtk_center_box_set_center_widget(self->center_box,
                                     srn_flow_controller_new(NULL));
}

static void
srn_window_constructed(GObject *object) {
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

    /* Load template from resource */
    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/window.ui");
    gtk_widget_class_bind_template_child(widget_class, SrnWindow, center_box);
}

/**
 * srn_window_new:
 * @app: A #SrnApplication.
 *
 * Returns: (transfer full): A new #SrnWindow.
 */
SrnWindow *
srn_window_new(SrnApplication *app) {
    return g_object_new(SRN_TYPE_WINDOW,
                        "application", GTK_APPLICATION(app),
                        NULL);
}
