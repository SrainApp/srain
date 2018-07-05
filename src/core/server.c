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

#include "core/core.h"
#include "sirc/sirc.h"
#include "config/reader.h"
#include "meta.h"
#include "srain.h"
#include "log.h"
#include "utils.h"
#include "i18n.h"

SrnServer* srn_server_new(const char *name, SrnServerConfig *cfg){
    SrnServer *srv;

    g_return_val_if_fail(name, NULL);
    g_return_val_if_fail(RET_IS_OK(srn_server_config_check(cfg)), NULL);

    srv = g_malloc0(sizeof(SrnServer));

    str_assign(&srv->name, name);
    srv->cfg = cfg;
    srv->addr = cfg->addrs->data;

    srv->state = SRN_SERVER_STATE_DISCONNECTED;
    srv->last_action = SRN_SERVER_ACTION_DISCONNECT; // It should be OK
    srv->negotiated = FALSE;
    srv->registered = FALSE;

    srv->cap = srn_server_cap_new();
    srv->cap->srv = srv;

    /* NOTE: Ping related issuses are not handled in server.c */
    srv->reconn_interval = SRN_SERVER_RECONN_INTERVAL;
    /* srv->last_pong = 0; */ // by g_malloc0()
    /* srv->delay = 0; */ // by g_malloc0()
    /* srv->ping_timer = 0; */ // by g_malloc0()
    /* srv->reconn_timer = 0; */ // by g_malloc0()

    /* Server user */
    srv->user_table = g_hash_table_new_full(
            g_str_hash, g_str_equal,
            NULL, (GDestroyNotify)srn_server_user_free);
    srv->_user = srn_server_add_and_get_user(srv, "");
    srv->user = srn_server_add_and_get_user(srv, srv->cfg->user->nick);
    srn_server_user_set_username(srv->user, srv->cfg->user->username);
    srn_server_user_set_realname(srv->user, srv->cfg->user->realname);
    srn_server_user_set_is_me(srv->user, TRUE);

    /* sirc */
    srv->irc = sirc_new_session(
            &srn_application_get_default()->irc_events,
            cfg->irc);
    if (!srv->irc) goto bad;
    sirc_set_ctx(srv->irc, srv);

    return srv;


bad:
    srn_server_free(srv);
    return NULL;
}

/**
 * @brief ``server_free`` frees a disconnected server.
 *
 * @param srv
 */
void srn_server_free(SrnServer *srv){
    g_return_if_fail(!srn_server_is_valid(srv));
    g_return_if_fail(srv->state == SRN_SERVER_STATE_DISCONNECTED);

    sirc_free_session(srv->irc);

    g_slist_free_full(srv->chat_list, (GDestroyNotify)srn_chat_free);
    // Server's chat should be freed after all chat in chat list are freed
    srn_chat_free(srv->chat);

    // srv->user and srv->_user are freed here as well
    g_hash_table_remove_all(srv->user_table);

    srn_server_cap_free(srv->cap);

    str_assign(&srv->name, NULL);

    g_free(srv);
}

void srn_server_set_config(SrnServer *srv, SrnServerConfig *cfg){
    sirc_set_config(srv->irc, cfg->irc);

    srv->cfg = cfg;
    srv->addr = cfg->addrs->data;
}

SrnRet srn_server_reload_config(SrnServer *srv){
    GSList *lst;
    SrnRet ret;
    SrnApplication *app;

    app = srn_application_get_default();
    lst = srv->chat_list;
    while (lst){
        SrnChat *chat;
        SrnChatConfig *chat_cfg;
        SrnChatConfig *old_chat_cfg;

        chat = lst->data;
        old_chat_cfg = chat->cfg;
        chat_cfg = srn_chat_config_new();
        ret = srn_config_manager_read_chat_config(
                app->cfg_mgr, chat_cfg, srv->name, chat->name);
        if (!RET_IS_OK(ret)){
            goto ERR;
        }
        ret = srn_chat_config_check(chat_cfg);
        if (!RET_IS_OK(ret)){
            goto ERR;
        }
        srn_chat_set_config(chat, chat_cfg);
        srn_chat_config_free(old_chat_cfg);

        lst = g_slist_next(lst);
        continue;
ERR:
        srn_chat_config_free(chat_cfg);
        return RET_ERR(_("Failed to chat config \"%1$s\" of server config \"%2$s\": %3$s"),
                chat->name, srv->name, RET_MSG(ret));
    }

    return SRN_OK;
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

SrnRet srn_server_quit(SrnServer *srv, const char *reason){
    // FIXME: reason is ignored
    return srn_server_state_transfrom(srv, SRN_SERVER_ACTION_QUIT);
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
        if (g_ascii_strcasecmp(chat->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    chat_cfg = srn_chat_config_new();
    ret = srn_config_manager_read_chat_config(
            srn_application_get_default()->cfg_mgr,
            chat_cfg, srv->name, name);
    if (!RET_IS_OK(ret)) {
        return ret;
    }
    ret = srn_chat_config_check(chat_cfg);
    if (!RET_IS_OK(ret)) {
        return ret;
    }

    /* If server's chat is not yet created and the chat name as same as
     * server name, create the server chat
     */
    if (!srv->chat && g_strcmp0(name, srv->name) == 0) {
        chat = srn_chat_new(srv, name, SRN_CHAT_TYPE_SERVER, chat_cfg);
        srv->chat = chat;
    } else {
        chat = srn_chat_new(srv, name,
                sirc_target_is_channel(srv->irc, name) ?
                SRN_CHAT_TYPE_CHANNEL : SRN_CHAT_TYPE_DIALOG,
                chat_cfg);
        srv->chat_list = g_slist_append(srv->chat_list, chat);
    }

    return SRN_OK;
}

SrnRet srn_server_rm_chat(SrnServer *srv, SrnChat *chat){
    GSList *lst;
    SrnChatConfig *chat_cfg;

    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(!chat->is_joined, SRN_ERR);

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
        if (g_ascii_strcasecmp(chat->name, name) == 0){
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

SrnChat* srn_server_add_and_get_chat(SrnServer *srv, const char *name){
    srn_server_add_chat(srv, name);
    return srn_server_get_chat(srv, name);
}

SrnRet srn_server_add_user(SrnServer *srv, const char *nick){
    SrnServerUser *user;

    if (srn_server_get_user(srv, nick)) {
        return SRN_ERR;
    }
    user = srn_server_user_new(srv, nick);
    return g_hash_table_insert(srv->user_table, user->nick, user) ?
        SRN_OK : SRN_ERR;
}

SrnServerUser* srn_server_get_user(SrnServer *srv, const char *nick){
    return g_hash_table_lookup(srv->user_table, nick);
}

SrnServerUser* srn_server_add_and_get_user(SrnServer *srv, const char *nick){
    srn_server_add_user(srv, nick);
    return srn_server_get_user(srv, nick);
}

SrnRet srn_server_rm_user(SrnServer *srv, SrnServerUser *user){
    return g_hash_table_remove(srv->user_table, user->nick) ? SRN_OK : SRN_ERR;
}

SrnRet srn_server_rename_user(SrnServer *srv, SrnServerUser *user,
        const char *nick){
    if (!g_hash_table_steal(srv->user_table, user->nick)){
        return SRN_ERR;
    }
    srn_server_user_set_nick(user, nick);
    return g_hash_table_insert(srv->user_table, user->nick, user) ?
        SRN_OK : SRN_ERR;
}
