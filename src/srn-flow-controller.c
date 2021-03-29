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

#include <glib/gi18n.h>

#include "srn-flow-controller.h"

struct _SrnFlowController {
    GtkGrid parent;

    SrnFlow *flow;

    GtkStack *stack;
};

/*********************
 * GObject functions *
 *********************/

enum {
    PROP_0,
    PROP_FLOW,
    N_PROPERTIES
};

G_DEFINE_TYPE(SrnFlowController, srn_flow_controller, GTK_TYPE_GRID);

static GParamSpec *obj_properties[N_PROPERTIES] = { };

static void
srn_flow_controller_set_property(GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec) {
    SrnFlowController *self = SRN_FLOW_CONTROLLER(object);

    switch (property_id) {
    case PROP_FLOW:
        self->flow = SRN_FLOW(g_value_dup_object(value));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_flow_controller_get_property(GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec) {
    SrnFlowController *self = SRN_FLOW_CONTROLLER(object);

    switch (property_id) {
    case PROP_FLOW:
        g_value_set_object(value, srn_flow_controller_get_flow(self));
        break;
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_flow_controller_init(SrnFlowController *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void
srn_flow_controller_constructed(GObject *object) {
    G_OBJECT_CLASS(srn_flow_controller_parent_class)->constructed(object);
}

static void
srn_flow_controller_finalize(GObject *object) {
    SrnFlowController *self = SRN_FLOW_CONTROLLER(object);

    g_clear_object(&self->flow);

    G_OBJECT_CLASS(srn_flow_controller_parent_class)->finalize(object);
}

static void
srn_flow_controller_class_init(SrnFlowControllerClass *class) {
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_flow_controller_constructed;
    object_class->finalize = srn_flow_controller_finalize;
    object_class->set_property = srn_flow_controller_set_property;
    object_class->get_property = srn_flow_controller_get_property;

    /* Install properties */
    obj_properties[PROP_FLOW] =
        g_param_spec_object("flow",
                            N_("Flow"),
                            N_("Flow controlled by controller"),
                            SRN_TYPE_FLOW,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_properties(object_class,
                                      N_PROPERTIES,
                                      obj_properties);

    /* Load template from resource */
    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/flow-controller.ui");
    gtk_widget_class_bind_template_child(widget_class, SrnFlowController, stack);
}


/**
 * srn_flow_controller_new:
 * @flow: A #SrnFlow.
 *
 * Allocate a new #SrnFlowController for given @flow.
 *
 * Returns: (transfer full): A new allocated #SrnFlowController.
 */
SrnFlowController *
srn_flow_controller_new(SrnFlow *flow) {
    return SRN_FLOW_CONTROLLER(g_object_new(SRN_TYPE_FLOW_CONTROLLER,
                                            "flow", flow,
                                            NULL));
}

/**
 * srn_flow_controller_get_flow:
 * @self: A #SrnFlowController.
 *
 * Get #SrnFlow of @self.
 *
 * Returns: (transfer none): The #SrnFlow of controller.
 */
SrnFlow *
srn_flow_controller_get_flow(SrnFlowController *self) {
    return self->flow;
}
