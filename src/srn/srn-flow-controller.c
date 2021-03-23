/* Copyright (C) 2016-2021 Shengyu Zhang <i@silverrainz.me>
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

#include "sif/sif.h"

#include "srn-flow-controller.h"

struct _SrnFlowController {
    GtkApplication parent;
};

/*********************
 * GObject functions *
 *********************/

enum {
    PROP_0,
    N_PROPERTIES
};

G_DEFINE_TYPE(SrnFlowController, srn_flow_controller, G_TYPE_OBJECT);

static GParamSpec *obj_properties[N_PROPERTIES] = { };

static void
srn_flow_controller_set_property(GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_flow_controller_get_property(GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_flow_controller_init(SrnFlowController *self) {
}

static void
srn_flow_controller_constructed(GObject *object) {
    G_OBJECT_CLASS(srn_flow_controller_parent_class)->constructed(object);
}

static void
srn_flow_controller_finalize(GObject *object) {
    G_OBJECT_CLASS(srn_flow_controller_parent_class)->finalize(object);
}

static void
srn_flow_controller_class_init(SrnFlowControllerClass *class) {
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_flow_controller_constructed;
    object_class->finalize = srn_flow_controller_finalize;
    object_class->set_property = srn_flow_controller_set_property;
    object_class->get_property = srn_flow_controller_get_property;

    /* Install properties */
    g_object_class_install_properties(object_class,
                                      N_PROPERTIES,
                                      obj_properties);
}
