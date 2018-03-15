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
 * @file server_ui_event.c
 * @brief Server UI event callbacks
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-07-19
 */

#include <string.h>

#include "core/core.h"
#include "sirc/sirc.h"
#include "server_ui_event.h"
#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "rc.h"
#include "filter.h"
#include "config/config.h"
#include "ret.h"
#include "utils.h"

static Server* ctx_get_server(SuiSession *sui);
static Chat* ctx_get_chat(SuiSession *sui);

SrnRet server_ui_event_open(SuiApplication *app, SuiEvent event, GVariantDict *params){
    // TODO: config
    return SRN_OK;
}

SrnRet server_ui_event_activate(SuiApplication *app, SuiEvent event, GVariantDict *params){
    SrnApplication *srn_app;

    srn_app = sui_application_get_ctx(app);
    sui_new_window(app, &srn_app->ui_win_events, sui_window_config_new());
    return SRN_OK;
}

SrnRet server_ui_event_shutdown(SuiApplication *app, SuiEvent event, GVariantDict *params){
    return SRN_OK;
}

SrnRet server_ui_event_connect(SuiWindow *win, SuiEvent event, GVariantDict *params){
    const char *name = NULL;
    const char *nick = NULL;
    const char *realname= "";
    const char *username = "";
    SrnRet ret = SRN_ERR;
    SrnApplication *app;
    Server *srv = NULL;
    ServerPrefs *prefs = NULL;

    app = srn_application_get_default();

    g_variant_dict_lookup(params, "name", SUI_EVENT_PARAM_STRING, &name);
    if (!str_is_empty(name)){
        /* If params "name" is not specified, connecting to predefined server */
        prefs = srn_application_get_server_config(app, name);
        if (prefs->srv){
            return RET_ERR(_("Server \"%1$s\" already exists"), name);
        }
    } else {
        /* Else, it means that user is trying to connect to a custom server */
        const char *host = NULL;
        int port = 0;
        const char *passwd = NULL;
        const char *encoding = "utf-8"; // TODO
        gboolean tls = FALSE;
        gboolean tls_noverify = FALSE;

        g_variant_dict_lookup(params, "host", SUI_EVENT_PARAM_STRING, &host);
        g_variant_dict_lookup(params, "port", SUI_EVENT_PARAM_INT, &port);
        g_variant_dict_lookup(params, "password", SUI_EVENT_PARAM_STRING, &passwd);
        g_variant_dict_lookup(params, "tls", SUI_EVENT_PARAM_BOOL, &tls);
        g_variant_dict_lookup(params, "tls-noverify", SUI_EVENT_PARAM_BOOL, &tls_noverify);

        /* Create ServerPrefs */
        prefs = server_prefs_new_from_basename(host);
        if (!prefs) {
            return RET_ERR(_("Failed to create server \"%1$s\""), host);
        }
        ret = srn_config_manager_read_server_config(
                srn_application_get_default()->cfg_mgr,
                prefs,
                host);
        if (!RET_IS_OK(ret)){
            server_prefs_free(prefs);
            return ret;
        }

        // FIXME: config
        // prefs->port = port;
        // if (!str_is_empty(host)){
            // str_assign(&prefs->host, host);
        // }
        if (!str_is_empty(passwd)){
            str_assign(&prefs->passwd, passwd);
        }
        if (!str_is_empty(encoding)){
            str_assign(&prefs->irc->encoding, encoding);
        }
        prefs->irc->tls = tls;
        prefs->irc->tls_noverify = tls_noverify;
    }

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);
    g_variant_dict_lookup(params, "username", SUI_EVENT_PARAM_STRING, &username);
    g_variant_dict_lookup(params, "realname", SUI_EVENT_PARAM_STRING, &realname);

    if (!str_is_empty(nick)){
        str_assign(&prefs->nickname, nick);
    }
    if (!str_is_empty(username)){
        str_assign(&prefs->username, username);
    }
    if (!str_is_empty(realname)){
        str_assign(&prefs->realname, realname);
    }

    ret = server_prefs_check(prefs);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    /* Create Server */
    srv = srn_application_add_server(app, prefs->name);
    if (!srv) {
        SrnRet ret;

        ret = RET_ERR(_("Failed to instantiate server \"%1$s\""), prefs->name);
        if (str_is_empty(name)){
            server_prefs_free(prefs);
        }
        return ret;
    }

    return server_connect(srv);
}

SrnRet server_ui_event_server_list(SuiWindow *win, SuiEvent event,
        GVariantDict *params){
    // FIXME: dirty hack
    GSList *lst;
    SrnApplication *app;

    app = srn_application_get_default();
    lst = app->srv_cfg_list;
    while (lst){
        ServerPrefs *prefs = lst->data;
        if (prefs->predefined){
            sui_server_list_add(prefs->name);
        }
        lst = g_slist_next(lst);
    }

    return SRN_OK;
}

SrnRet server_ui_event_disconnect(SuiSession *sui, SuiEvent event, GVariantDict *params){
    SrnRet ret;
    Server *srv;
    ServerState prev_state;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    prev_state = srv->state;
    ret = server_disconnect(srv);
    if (!RET_IS_OK(ret)){
        chat_add_error_message(chat, chat->user->nick, RET_MSG(ret));
    }

    if (prev_state == SERVER_STATE_RECONNECTING){
        chat_add_misc_message(chat, chat->user->nick, _("Reconnection stopped"));
    }

    return SRN_OK;
}

SrnRet server_ui_event_quit(SuiSession *sui, SuiEvent event, GVariantDict *params){
    SrnRet ret;
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    if (server_is_registered(srv)) {
        ret = sirc_cmd_quit(srv->irc, srv->prefs->quit_message);
    } else {
        ret = server_state_transfrom(srv, SERVER_ACTION_QUIT);
        if (!server_is_valid(srv)){
            return SRN_OK;
        }
    }

    if (!RET_IS_OK(ret)){
        chat_add_error_message(chat, chat->user->nick,
                _("Failed to quit from server, try again please"));
    }

    return ret;
}

SrnRet server_ui_event_send(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *msg = NULL;
    SrnRet ret = SRN_ERR;
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "message", SUI_EVENT_PARAM_STRING, &msg);
    g_return_val_if_fail(!str_is_empty(msg), SRN_ERR);

    // Command or message?
    if (msg[0] == '/'){
        ret = server_cmd(chat, msg);
        // FIXME: chat may be invalid now
        if (!server_is_valid(srv)){
            return SRN_ERR;
        }
        if (RET_IS_OK(ret)){
            if (ret != SRN_OK) { // Has OK message
                chat_add_misc_message(chat, chat->user->nick, RET_MSG(ret));
            }
        } else {
            chat_add_error_message(chat, chat->user->nick, RET_MSG(ret));
        }
    } else {
        if (chat == chat->srv->chat) {
            ret = RET_ERR(_("Can not send message to a server"));
            chat_add_error_message(chat, chat->user->nick, RET_MSG(ret));
            return ret;
        }

        chat_add_sent_message(chat, msg); // Show on UI first

        ret = sirc_cmd_msg(chat->srv->irc, chat->name, msg);
        if (!RET_IS_OK(ret)){
            chat_add_error_message_fmt(chat, chat->user->nick,
                    _("Failed to send message: %1$s"), RET_MSG(ret));
        }
    }

    return ret;
}

SrnRet server_ui_event_join(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *chan = NULL;
    const char *passwd = NULL;
    Server *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    g_variant_dict_lookup(params, "channel", SUI_EVENT_PARAM_STRING, &chan);
    g_variant_dict_lookup(params, "password", SUI_EVENT_PARAM_STRING, &passwd);

    return sirc_cmd_join(srv->irc, chan, passwd);
}

SrnRet server_ui_event_part(SuiSession *sui, SuiEvent event, GVariantDict *params){
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    if (chat->joined) {
        return sirc_cmd_part(srv->irc, chat->name, "Leave.");
    } else {
        return server_rm_chat(srv, chat);
    }
}

SrnRet server_ui_event_query(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    Server *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return server_add_chat(srv, nick);;
}

SrnRet server_ui_event_unquery(SuiSession *sui, SuiEvent event, GVariantDict *params){
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    return server_rm_chat(srv, chat);
}

SrnRet server_ui_event_kick(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return sirc_cmd_kick(srv->irc, nick, chat->name, "Kick.");
}

SrnRet server_ui_event_invite(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return sirc_cmd_invite(srv->irc, nick, chat->name);
}

SrnRet server_ui_event_whois(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    Server *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return sirc_cmd_whois(srv->irc, nick);
}

SrnRet server_ui_event_ignore(SuiSession *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    Chat *chat;

    chat = ctx_get_chat(sui);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return nick_filter_add_nick(chat, nick);
}

SrnRet server_ui_event_cutover(SuiSession *sui, SuiEvent event, GVariantDict *params){
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    // FIXME: server has not held by ServerPrefs now
    // g_return_val_if_fail(server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(chat, SRN_ERR);

    srv->cur_chat = chat;

    return SRN_OK;
}

SrnRet server_ui_event_chan_list(SuiSession *sui, SuiEvent event, GVariantDict *params){
    Server *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    return sirc_cmd_list(srv->irc, NULL, NULL);
}

/* Get a Server object from SuiSession context (sui->ctx) */
static Server* ctx_get_server(SuiSession *sui){
    void *ctx;
    Chat *chat;

    ctx = sui_get_ctx(sui);
    g_return_val_if_fail(ctx, NULL);

    chat = ctx;

    return chat->srv;
}

/* Get a Chat object from SuiSession context (sui->ctx) */
static Chat* ctx_get_chat(SuiSession *sui){
    return sui_get_ctx(sui);
}
