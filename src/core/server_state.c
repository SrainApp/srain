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
 * @file server_state.c
 * @brief Server connection state control
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.3
 * @date 2018-01-22
 */

#include <stdio.h>
#include <glib.h>

#include "core/core.h"
#include "sirc/sirc.h"

#include "srain.h"
#include "log.h"
#include "utils.h"
#include "i18n.h"

static const char *srn_server_state_to_string(SrnServerState state);
static const char *srn_server_action_to_string(SrnServerAction action);
static gboolean srn_server_reconnect_timeout(gpointer user_data);

/**
 * @brief server_state_transfrom SrnServer's connection state macheine, accept a
 *      action and transform the server to next state
 *
 * @param srv
 * @param action
 *
 * @return SRN_OK if transformation succes
 *
 * NOTE: The server may be freed in this function, check it carefully.
 */
SrnRet srn_server_state_transfrom(SrnServer *srv, SrnServerAction action){
    bool free;
    const char *unallowed;
    SrnRet ret;
    SrnServerState cur_state;
    SrnServerState next_state;

    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    free = FALSE;
    unallowed = _("Unallowed action: %1$s");
    ret = SRN_OK;
    next_state = cur_state = srv->state;

    /* State machine starts */
    switch (srv->state) {
        case SRN_SERVER_STATE_DISCONNECTED:
            switch (action) {
                case SRN_SERVER_ACTION_CONNECT:
                    sirc_connect(srv->irc, srv->addr->host, srv->addr->port);
                    next_state = SRN_SERVER_STATE_CONNECTING;
                    break;
                case SRN_SERVER_ACTION_DISCONNECT:
                    ret = RET_ERR(unallowed, _("Server is already disconnected"));
                    break;
                case SRN_SERVER_ACTION_QUIT:
                    free = TRUE;
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                default:
                    ret = SRN_ERR;
                    g_warn_if_reached();
            }
            break;

        case SRN_SERVER_STATE_CONNECTING:
            switch (action) {
                case SRN_SERVER_ACTION_CONNECT:
                    ret = RET_ERR(unallowed, _("Server is connecting, please don't repeat connect"));
                    break;
                case SRN_SERVER_ACTION_CONNECT_FAIL:
                    srv->reconn_timer = g_timeout_add(srv->reconn_interval,
                            srn_server_reconnect_timeout, srv);
                    next_state = SRN_SERVER_STATE_RECONNECTING;
                    break;
                case SRN_SERVER_ACTION_CONNECT_FINISH:
                    // TODO: reset reconn_interval after connection becomes stable
                    srv->reconn_interval = SRN_SERVER_RECONN_STEP;
                    next_state = SRN_SERVER_STATE_CONNECTED;
                    break;
                case SRN_SERVER_ACTION_DISCONNECT:
                    sirc_cancel_connect(srv->irc);
                    next_state = SRN_SERVER_STATE_DISCONNECTING;
                    break;
                case SRN_SERVER_ACTION_QUIT:
                    sirc_cancel_connect(srv->irc);
                    next_state = SRN_SERVER_STATE_QUITING;
                    break;
                default:
                    ret = SRN_ERR;
                    g_warn_if_reached();
            }
            break;

        case SRN_SERVER_STATE_CONNECTED:
            switch (action) {
                case SRN_SERVER_ACTION_CONNECT:
                    ret = RET_ERR(unallowed, _("Server is already connected"));
                    break;
                case SRN_SERVER_ACTION_DISCONNECT: // Connection closed by local
                    sirc_disconnect(srv->irc);
                    next_state = SRN_SERVER_STATE_DISCONNECTING;
                    break;
                case SRN_SERVER_ACTION_RECONNECT: // Ping time out
                    sirc_disconnect(srv->irc);
                    next_state = SRN_SERVER_STATE_CONNECTED; // Keep state
                    break;
                case SRN_SERVER_ACTION_QUIT: // User received a QUIT command sent by self
                    sirc_disconnect(srv->irc);
                    next_state = SRN_SERVER_STATE_QUITING;
                    break;
                case SRN_SERVER_ACTION_DISCONNECT_FINISH:
                    srv->reconn_timer = g_timeout_add(srv->reconn_interval,
                            srn_server_reconnect_timeout, srv);
                    next_state = SRN_SERVER_STATE_RECONNECTING;
                    break;
                default:
                    ret = SRN_ERR;
                    g_warn_if_reached();
            }
            break;

        case SRN_SERVER_STATE_DISCONNECTING:
            switch (action) {
                case SRN_SERVER_ACTION_CONNECT:
                    ret = RET_ERR(unallowed, _("Server is disconnecting"));
                    break;
                case SRN_SERVER_ACTION_CONNECT_FAIL:
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                 case SRN_SERVER_ACTION_DISCONNECT:
                     sirc_cancel_connect(srv->irc); // Force disconnect
                     next_state = SRN_SERVER_STATE_DISCONNECTING; // Keep state
                     break;
                 case SRN_SERVER_ACTION_QUIT:
                     sirc_cancel_connect(srv->irc); // Force quit
                     next_state = SRN_SERVER_STATE_QUITING;
                     break;
                case SRN_SERVER_ACTION_DISCONNECT_FINISH:
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                default:
                    ret = SRN_ERR;
                    g_warn_if_reached();
            }
            break;

        case SRN_SERVER_STATE_QUITING:
            switch (action) {
                case SRN_SERVER_ACTION_CONNECT:
                    ret = RET_ERR(unallowed, _("Server is quiting"));
                    break;
                case SRN_SERVER_ACTION_CONNECT_FAIL:
                    free = TRUE;
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                case SRN_SERVER_ACTION_DISCONNECT:
                    ret = RET_ERR(unallowed, _("Server is quiting"));
                    break;
                case SRN_SERVER_ACTION_QUIT: // Force quit
                    sirc_cancel_connect(srv->irc);
                    next_state = SRN_SERVER_STATE_QUITING; // Keep state
                    break;
                case SRN_SERVER_ACTION_DISCONNECT_FINISH:
                    free = TRUE;
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                default:
                    ret = SRN_ERR;
                    g_warn_if_reached();
            }
            break;

        case SRN_SERVER_STATE_RECONNECTING:
            switch (action) {
                case SRN_SERVER_ACTION_CONNECT:
                    sirc_connect(srv->irc, srv->addr->host, srv->addr->port);
                    next_state = SRN_SERVER_STATE_CONNECTING;
                    break;
                case SRN_SERVER_ACTION_DISCONNECT:
                    g_source_remove(srv->reconn_timer);
                    srv->reconn_timer = 0;
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                case SRN_SERVER_ACTION_QUIT:
                    g_source_remove(srv->reconn_timer);
                    srv->reconn_timer = 0;
                    free = TRUE;
                    next_state = SRN_SERVER_STATE_DISCONNECTED;
                    break;
                default:
                    ret = SRN_ERR;
                    g_warn_if_reached();
            }
            break;

        default:
            ret = SRN_ERR;
            g_warn_if_reached();
    }

    if (RET_IS_OK(ret)){
        LOG_FR("Server %s: %s + %s -> %s",
                srv->cfg->name,
                srn_server_state_to_string(cur_state),
                srn_server_action_to_string(action),
                srn_server_state_to_string(next_state));
        srv->state = next_state;
        srv->last_action = action;
    } else {
        WARN_FR("Server %s: %s + %s -> error: %s",
                srv->cfg->name,
                srn_server_state_to_string(cur_state),
                srn_server_action_to_string(action),
                RET_MSG(ret));
    }

    if (free){ // The server should be free now, be careful
        srn_server_free(srv);
    }

    return ret;
}

static const char *srn_server_state_to_string(SrnServerState state){
    switch (state) {
        case SRN_SERVER_STATE_CONNECTING:
            return "SRN_SERVER_STATE_CONNECTING";
        case SRN_SERVER_STATE_CONNECTED:
            return "SRN_SERVER_STATE_CONNECTED";
        case SRN_SERVER_STATE_DISCONNECTING:
            return "SRN_SERVER_STATE_DISCONNECTING";
        case SRN_SERVER_STATE_QUITING:
            return "SRN_SERVER_STATE_QUITING";
        case SRN_SERVER_STATE_RECONNECTING:
            return "SRN_SERVER_STATE_RECONNECTING";
        case SRN_SERVER_STATE_DISCONNECTED:
            return "SRN_SERVER_STATE_DISCONNECTED";
        default:
            g_warn_if_reached();
            return NULL;
    }
}

static const char *srn_server_action_to_string(SrnServerAction action){
    switch (action) {
        case SRN_SERVER_ACTION_CONNECT:
            return "SRN_SERVER_ACTION_CONNECT";
        case SRN_SERVER_ACTION_CONNECT_FAIL:
            return "SRN_SERVER_ACTION_CONNECT_FAIL";
        case SRN_SERVER_ACTION_CONNECT_FINISH:
            return "SRN_SERVER_ACTION_CONNECT_FINISH";
        case SRN_SERVER_ACTION_DISCONNECT:
            return "SRN_SERVER_ACTION_DISCONNECT";
        case SRN_SERVER_ACTION_QUIT:
            return "SRN_SERVER_ACTION_QUIT";
        case SRN_SERVER_ACTION_RECONNECT:
            return "SRN_SERVER_ACTION_RECONNECT";
        case SRN_SERVER_ACTION_DISCONNECT_FINISH:
            return "SRN_SERVER_ACTION_DISCONNECT_FINISH";
        default:
            g_warn_if_reached();
            return NULL;
    }
}

static gboolean srn_server_reconnect_timeout(gpointer user_data){
    SrnServer *srv;

    srv = user_data;
    srv->reconn_interval += SRN_SERVER_RECONN_STEP;
    srn_server_state_transfrom(srv, SRN_SERVER_ACTION_CONNECT);

    return G_SOURCE_REMOVE;
}
