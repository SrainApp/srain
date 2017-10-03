/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-07-19
 */

#include <string.h>

#include "server.h"
#include "server_cmd.h"
#include "server_ui_event.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "rc.h"
#include "filter.h"
#include "prefs.h"
#include "ret.h"
#include "utils.h"

static Server* ctx_get_server(SuiSession *sui);
static Chat* ctx_get_chat(SuiSession *sui);

SrnRet server_ui_event_open(SuiEvent event, GVariantDict *params){
    int len;
    char **urls;
    SrnRet ret = SRN_OK;

    g_variant_dict_lookup(params, "urls", SUI_EVENT_PARAM_STRINGS, &urls);
    len =  g_strv_length(urls);

    for (int i = 0; i < len; i ++){
        ret = server_url_open(urls[i]);
        if (!RET_IS_OK(ret)){
            return ret;
        }
    }

    return ret;
}

SrnRet server_ui_event_activate(SuiEvent event, GVariantDict *params){
    return rc_read();
}

SrnRet server_ui_event_connect(SuiEvent event, GVariantDict *params){
    const char *name = NULL;
    const char *host = NULL;
    int port = 0;
    const char *passwd = NULL;
    const char *encoding = "UTF-8"; // TODO
    const char *nick = NULL;
    const char *realname= "";
    const char *username = PACKAGE_NAME; // TODO
    gboolean tls = FALSE;
    gboolean tls_noverify = FALSE;
    SrnRet ret = SRN_ERR;
    Server *srv;

    g_variant_dict_lookup(params, "name", SUI_EVENT_PARAM_STRING, &name);
    g_variant_dict_lookup(params, "host", SUI_EVENT_PARAM_STRING, &host);
    g_variant_dict_lookup(params, "port", SUI_EVENT_PARAM_INT, &port);
    g_variant_dict_lookup(params, "password", SUI_EVENT_PARAM_STRING, &passwd);
    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);
    g_variant_dict_lookup(params, "realname", SUI_EVENT_PARAM_STRING, &realname);
    g_variant_dict_lookup(params, "tls", SUI_EVENT_PARAM_BOOL, &tls);
    g_variant_dict_lookup(params, "tls-noverify", SUI_EVENT_PARAM_BOOL, &tls_noverify);

    if (str_is_empty(name)){
        return RET_ERR(_("You must specified a server name"));
    }

    ServerPrefs *prefs = server_prefs_new(name);
    if (!prefs){
        return RET_ERR(_("Server already exist: %s"));
    }

    ret = prefs_read_server_prefs(prefs);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    prefs->port = port;
    if (!str_is_empty(host)){
        str_assign(&prefs->host, host);
    }
    if (!str_is_empty(passwd)){
        str_assign(&prefs->passwd, passwd);
    }
    if (!str_is_empty(encoding)){
        str_assign(&prefs->encoding, encoding);
    }
    if (!str_is_empty(nick)){
        str_assign(&prefs->nickname, nick);
    }
    if (!str_is_empty(username)){
        str_assign(&prefs->username, username);
    }
    if (!str_is_empty(realname)){
        str_assign(&prefs->realname, realname);
    }

    prefs->irc->tls = tls;
    prefs->irc->tls_noverify = tls_noverify;

    ret = server_prefs_check(prefs);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    srv = server_new_from_prefs(prefs);
    if (!srv) {
        return RET_ERR(_("Failed to instantiate server \"%s\""), prefs->name);
    }

    return server_connect(srv);
}

SrnRet server_ui_event_server_list(SuiEvent event, GVariantDict *params){
    // FIXME: dirty hack
    extern GSList *server_prefs_list;
    GSList *lst;

    lst = server_prefs_list;
    while (lst){
        ServerPrefs *prefs = lst->data;
        sui_server_list_add(prefs->name);
        lst = g_slist_next(lst);
    }

    return SRN_OK;
}

SrnRet server_ui_event_disconnect(SuiSession *sui, SuiEvent event, GVariantDict *params){
    SrnRet ret;
    Server *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(server_is_valid(srv), SRN_ERR);

    ret = sirc_cmd_quit(srv->irc, "QUIT");
    if (!RET_IS_OK(ret)){
        server_free(srv); // FIXME
        return SRN_ERR;
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

        ret = sirc_cmd_msg(chat->srv->irc, chat->name, msg);
        if (RET_IS_OK(ret)){
            chat_add_sent_message(chat, msg);
        } else {
            chat_add_error_message_fmt(chat, chat->user->nick,
                    _("Failed to send message: %s"), RET_MSG(ret));
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
        return server_rm_chat(srv, chat->name);
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

    return server_rm_chat(srv, chat->name);
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
