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
#include <gtk/gtk.h>
// For g_irepository_dump
#include <girepository.h>

#include "srn-flow.h"
// For package meta infos
#include "srn-meta.h"

/*********************
 * GObject functions *
 *********************/

G_DEFINE_INTERFACE(SrnFlow, srn_flow, G_TYPE_OBJECT)

enum {
    BUSY,
    NEXT,
    ABORT,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

static void
srn_flow_default_init(SrnFlowInterface *iface) {
    /* add properties and signals to the interface here */

    /**
     * SrnFlow::busy:
     * @flow: the object which received the signal
     * @text: (transfer none): the text message to indicate
     *        the busy state
     * @widget: (transfer none) (nullable): the widget to
     *          indicate the busy state
     *
     * This signal is emitted when flow comes into busy state.
     * The SrnFlowController will add the widget to interface to
     * indicate the busy state of this flow.
     * If the widget is NULL, a #GtkLabel with text will be created.
     */
    signals[BUSY] =
        g_signal_new(N_("busy"),
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     2 /* n_params */,
                     G_TYPE_STRING, /* param_types text */
                     GTK_TYPE_WIDGET /* param_types widget */);

    /**
     * SrnFlow::next:
     * @flow: the object which received the signal
     * @widget: (transfer none): the operation widget of
     *          the next step of flow
     *
     * This signal is emitted when flow comes into next
     * step. The SrnFlowController will add the widget to
     * user interface so that user can interact
     * with the it.
     */
    signals[NEXT] =
        g_signal_new(N_("next"),
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     1 /* n_params */,
                     GTK_TYPE_WIDGET /* param_types widget */);

    /**
     * SrnFlow::abort:
     * @flow: the object which received the signal
     * @text: (transfer none): the text message to
     *        indicate abort of the flow
     * @widget: (transfer none) (nullable): the widget
     *          to indicate abort of the flow
     *
     * This signal is emitted when flow aborts.
     * The SrnFlowController will add the widget to
     * interface to indicate the flow is aborted.
     * If the widget is NULL, a #GtkLabel with text
     * will be created.
     */
    signals[ABORT] =
        g_signal_new(N_("abort"),
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     2 /* n_params */,
                     G_TYPE_STRING, /* param_types text */
                     GTK_TYPE_WIDGET /* param_types widget */);
}
/**
 * srn_flow_launch:
 * @self: a #SrnFlow.
 *
 * Lanuch a flow.
 *
 * Returns: (nullable) (transfer none): Operation widget
 *           of the first step of flow.
 */
GtkWidget *
srn_flow_launch(SrnFlow *self, GCancellable *cancellable, GError **error) {
    SrnFlowInterface *iface;

    g_return_val_if_fail(SRN_IS_FLOW(self), NULL);
    g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    iface = SRN_FLOW_GET_IFACE(self);
    g_return_val_if_fail(iface->launch != NULL, NULL);
    return iface->launch(self, cancellable, error);
}
