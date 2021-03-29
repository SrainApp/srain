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

#include "srn-flow.h"


/**
 * SrnFlow:
 *
 * Interface for defineing a interactive flow.
 *
 * See also [class@FlowController].
 */

G_DEFINE_INTERFACE(SrnFlow, srn_flow, G_TYPE_OBJECT)

enum {
    BUSY,
    PREV,
    NEXT,
    ABORT,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

static void
srn_flow_default_init(SrnFlowInterface *iface) {
    /* add properties and signals to the interface here */

    /**
     * SrnFlow:name
     *
     * Name of the flow.
     */
    g_object_interface_install_property(iface,
                                        g_param_spec_string("name",
                                                N_("Name"),
                                                N_("Name of flow"),
                                                NULL,
                                                G_PARAM_READABLE));

    /**
     * SrnFlow::busy:
     * @flow: #SrnFlow implementation which received the signal
     * @caption: (transfer none): the text message to indicate
     *           the busy state
     * @indicator: (transfer none) (nullable): the widget to
     *          indicate the busy state
     * @cancel: (transfer none) (nullable): Object for cancellation
     *          of busy state.
     *
     * This signal is emitted when @flow comes into busy state.
     * The [class@FlowController] will add the @indicator to interface
     * and use @caption as title to indicate the busy state of this flow.
     * If the @indicator is NULL, a default indicator will be used.
     */
    signals[BUSY] =
        g_signal_new("busy",
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     3 /* n_params */,
                     G_TYPE_STRING /* param_types caption */,
                     GTK_TYPE_WIDGET /* param_types indicator */,
                     G_TYPE_CANCELLABLE /* param_types cancel */);

    /**
     * SrnFlow::prev:
     * @flow: #SrnFlow implementation which received the signal
     *
     * This signal is emitted when @flow asks [class@FlowController]
     * to return to previous step.
     */
    signals[PREV] =
        g_signal_new("prev",
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     0 /* n_params */);

    /**
     * SrnFlow::next:
     * @flow: #SrnFlow implementation which received the signal
     * @caption: (transfer none): the text message that descripts
     *           the @interator
     * @interator: (transfer none): the interactive widget that
     *             represents next step of @flow
     *
     * This signal is emitted when @flow comes into next
     * step. The [class@FlowController] will add the @interator to
     * user interface so that user can interact with the it.
     */
    signals[NEXT] =
        g_signal_new("next",
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     2 /* n_params */,
                     G_TYPE_STRING /* param_types caption */,
                     GTK_TYPE_WIDGET /* param_types interator */);

    /**
     * SrnFlow::abort:
     * @flow: #SrnFlow implementation which received the signal
     * @caption: (transfer none): the text message to
     *           indicate abort of the flow
     * @indicator: (transfer none) (nullable): the widget
     *          to indicate abort of the flow
     *
     * This signal is emitted when @flow asks [class@FlowController] to abort.
     * The @SrnFlowController will add the widget to interface to
     * indicate the @flow is aborted. If the widget is NULL,
     * a default indicator will be will be used.
     */
    signals[ABORT] =
        g_signal_new("abort",
                     SRN_TYPE_FLOW,
                     G_SIGNAL_RUN_LAST,
                     0, /* class offet */
                     NULL /* accumulator */,
                     NULL /* accumulator data */,
                     NULL /* C marshaller */,
                     G_TYPE_NONE /* return_type */,
                     2 /* n_params */,
                     G_TYPE_STRING, /* param_types caption */
                     GTK_TYPE_WIDGET /* param_types indicator */);
}

/**
 * srn_flow_launch:
 * @self:
 * @err:
 *
 * Lanuch a flow.
 *
 * Returns: TRUE if flow launched successfully.
 */
gboolean
srn_flow_launch(SrnFlow *self, GError **err) {
    SrnFlowInterface *iface;

    g_return_val_if_fail(SRN_IS_FLOW(self), FALSE);
    g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

    iface = SRN_FLOW_GET_IFACE(self);
    g_return_val_if_fail(iface->launch != NULL, FALSE);
    return iface->launch(self, err);
}

/**
 * srn_flow_get_name:
 * @self:
 *
 * See [property@Flow:name].
 *
 * Returns: (transfer none):
 */
const gchar *
srn_flow_get_name(SrnFlow *self) {
    const gchar *name;

    g_object_get(self,
                 "name", &name,
                 NULL);

    return name;
}
