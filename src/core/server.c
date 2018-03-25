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

/**
 * @file server.c
 * @brief
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-07-19
 */


#include <string.h>
#include <strings.h>
#include <gtk/gtk.h>

#include "core/core.h"
#include "sirc/sirc.h"
#include "config/reader.h"
#include "meta.h"
#include "srain.h"
#include "log.h"
#include "utils.h"
#include "i18n.h"

#include "server_irc_event.h"
#include "server_ui_event.h"

SrnServer* srn_server_new(SrnServerConfig *cfg){
    SrnServer *srv;

    srv = g_malloc0(sizeof(SrnServer));

    srv->state = SRN_SERVER_STATE_DISCONNECTED;
    srv->last_action = SRN_SERVER_ACTION_DISCONNECT; // It should be OK
    srv->negotiated = FALSE;
    srv->registered = FALSE;

    srv->addr = cfg->addrs->data;
    srv->cfg = cfg;
    srv->cap = srn_server_cap_new();
    srv->cap->srv = srv;

    /* NOTE: Ping related issuses are not handled in server.c */
    srv->reconn_interval = SRN_SERVER_RECONN_INTERVAL;
    /* srv->last_pong = 0; */ // by g_malloc0()
    /* srv->delay = 0; */ // by g_malloc0()
    /* srv->ping_timer = 0; */ // by g_malloc0()
    /* srv->reconn_timer = 0; */ // by g_malloc0()

    /* sirc */
    srv->irc = sirc_new_session(
            &srn_application_get_default()->irc_events,
            cfg->irc);
    if (!srv->irc) goto bad;
    sirc_set_ctx(srv->irc, srv);

    cfg->srv = srv;   // Link server to its cfg

    return srv;

bad:
    srn_server_free(srv);
    return NULL;
}

/**
 * @brief server_free Free a disconnected server, NEVER use this function
 *      directly on server which has been initialized
 *
 * @param srv
 */
void srn_server_free(SrnServer *srv){
    g_return_if_fail(srn_server_is_valid(srv));
    g_return_if_fail(srv->state == SRN_SERVER_STATE_DISCONNECTED);

    // srv->cfg is held by SrnApplication, just unlink it
    srv->cfg->srv = NULL;

    if (srv->cap) {
        srn_server_cap_free(srv->cap);
        srv->cap = NULL;
    }
    if (srv->user != NULL){
        srn_user_free(srv->user);
        srv->user = NULL;
    }
    if (srv->irc != NULL){
        sirc_free_session(srv->irc);
        srv->irc= NULL;
    }

    GSList *lst;
    /* Free chat list */
    if (srv->chat_list){
        lst = srv->chat_list;
        while (lst){
            if (lst->data){
                srn_chat_free(lst->data);
                lst->data = NULL;
            }
            lst = g_slist_next(lst);
        }
        g_slist_free(srv->chat_list);
        srv->chat_list = NULL;
    }

    /* Server's chat should be freed after all chat in chat list are freed */
    if (srv->chat != NULL){
        srn_chat_free(srv->chat);
        srv->chat = NULL;
    }

    g_free(srv);
}

bool srn_server_is_valid(SrnServer *srv){
    SrnApplication *app;

    app = srn_application_get_default();
    return srn_application_is_server_valid(app, srv);
}

/**
 * @brief server_connect Just an intuitive alias of a connect action
 *
 * @param srv
 *
 * @return
 */
SrnRet srn_server_connect(SrnServer *srv){
    return srn_server_state_transfrom(srv, SRN_SERVER_ACTION_CONNECT);
}

/**
 * @brief server_disconnect Just an intuitive alias of a disconnect action
 *
 * @param srv
 *
 * @return
 */
SrnRet srn_server_disconnect(SrnServer *srv){
    return srn_server_state_transfrom(srv, SRN_SERVER_ACTION_DISCONNECT);
}

/**
 * @brief server_is_registered Whether this server registered
 *
 * @param srv
 *
 * @return TRUE if registered
 */
bool srn_server_is_registered(SrnServer *srv){
    g_return_val_if_fail(srn_server_is_valid(srv), FALSE);

    return srv->state == SRN_SERVER_STATE_CONNECTED && srv->registered == TRUE;
}

void srn_server_wait_until_registered(SrnServer *srv){
    g_return_if_fail(srn_server_is_valid(srv));

    /* Waiting for connection established */
    while (srv->state == SRN_SERVER_STATE_CONNECTING) sui_proc_pending_event();
    /* Waiting until server registered */
    while (srv->state == SRN_SERVER_STATE_CONNECTED && srv->registered == FALSE)
        sui_proc_pending_event();
}

SrnRet srn_server_add_chat(SrnServer *srv, const char *name){
    GSList *lst;
    SrnRet ret;
    SrnChat *chat;
    SrnChatConfig *chat_cfg;

    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    chat_cfg = srn_chat_config_new();
    ret = srn_config_manager_read_chat_config(
            srn_application_get_default()->cfg_mgr,
            chat_cfg, srv->cfg->name, name);
    if (!RET_IS_OK(ret)) {
        return ret;
    }

    chat = srn_chat_new(srv, name, chat_cfg);
    // Creating server chat
    if (g_ascii_strcasecmp(name, META_SERVER) == 0) {
        srv->chat = chat;
        srv->user = srn_user_new(srv->chat,
                srv->cfg->nickname,
                srv->cfg->username,
                srv->cfg->realname,
                SRN_USER_CHIGUA);
        srv->chat->user = srn_user_ref(srv->user);
        srn_user_set_me(srv->user, TRUE);
    } else {
        srv->chat_list = g_slist_append(srv->chat_list, chat);
    }

    return SRN_OK;
}

SrnRet srn_server_rm_chat(SrnServer *srv, SrnChat *chat){
    GSList *lst;
    SrnChatConfig *chat_cfg;

    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(!chat->joined, SRN_ERR);

    lst = g_slist_find(srv->chat_list, chat);
    if (!lst) {
        return SRN_ERR;
    }

    if (srv->cur_chat == chat){
        srv->cur_chat = srv->chat;
    }
    chat_cfg = chat->cfg;
    srn_chat_free(chat);
    srn_chat_config_free(chat_cfg);
    srv->chat_list = g_slist_delete_link(srv->chat_list, lst);

    return SRN_OK;
}

SrnChat* srn_server_get_chat(SrnServer *srv, const char *name) {
    GSList *lst;
    SrnChat *chat;

    g_return_val_if_fail(srn_server_is_valid(srv), NULL);

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return chat;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}

/**
 * @brief server_get_chat_fallback
 *        This function never fail, if name is NULL or not chat found, return
 *        `srv->chat` instead.
 *
 * @param srv
 * @param name
 *
 * @return A instance of SrnChat
 */
SrnChat* srn_server_get_chat_fallback(SrnServer *srv, const char *name) {
    SrnChat *chat;

    g_return_val_if_fail(srn_server_is_valid(srv), NULL);

    if (!name || !(chat = srn_server_get_chat(srv, name))){
        chat = srv->chat;
    }

    return chat;
}
