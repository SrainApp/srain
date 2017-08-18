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
 * @version 0.06.1
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

void server_ui_event_open(SuiEvent event, const char *params[], int count){
    SrnRet ret = SRN_OK;

    for (int i = 0; i < count; i ++){
        ret = server_url_open(params[i]);
        if (!RET_IS_OK(ret)){
            sui_message_box(_("Error occurred while opening URL"), RET_MSG(ret));
        }
    }
}

void server_ui_event_activate(SuiEvent event, const char *params[], int count){
    SrnRet ret;

    ret = rc_read();
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error occurred while running commands"), RET_MSG(ret));
    }
}

void server_ui_event_connect(SuiEvent event, const char *params[], int count){
    SrnRet ret;
    Server *srv;

    g_return_if_fail(count == 9);
    const char *name = params[0];
    const char *host = params[1];
    int port = atoi(params[2]);
    const char *passwd = params[3];
    bool tls = strcmp(params[4], "TRUE") == 0 ? TRUE : FALSE;
    bool notverify = strcmp(params[5], "TRUE") == 0 ? TRUE : FALSE;
    const char *encoding = params[6];
    const char *nick = params[7];
    const char *realname= params[8];
    const char *username = PACKAGE_NAME;

    if (str_is_empty(name)){
        sui_message_box(_("Create server failed"), _("You must specified a server name"));
        return;
    }

    ServerPrefs *prefs = server_prefs_new(name);
    if (!prefs){
        char *errmsg = g_strdup_printf(_("Server already exist: %s"), name);
        sui_message_box(_("Create server failed"), errmsg);
        g_free(errmsg);
        return;
    }

    ret = prefs_read_server_prefs(prefs);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Read server prefs failed"), RET_MSG(ret));
        return;
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
    prefs->irc->tls_not_verify = notverify;

    ret = server_prefs_check(prefs);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Create server failed"), RET_MSG(ret));
        return;
    }

    srv = server_new_from_prefs(prefs);
    if (!srv) {
        ret = RET_ERR(_("Failed to instantiate server \"%s\""), prefs->name);
        sui_message_box(_("Create server failed"), RET_MSG(ret));
        return;
    }

    server_connect(srv);
}

void server_ui_event_disconnect(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;

    g_return_if_fail(count == 0);

    srv = ctx_get_server(sui);
    g_return_if_fail(server_is_valid(srv));

    if (!RET_IS_OK(sirc_cmd_quit(srv->irc, "QUIT"))){
        server_free(srv); // FIXME
    }
}

void server_ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *msg;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    msg = params[0];

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(chat);

    // Command or message?
    if (msg[0] == '/'){
        SrnRet ret;
        ret = server_cmd(chat, msg);
        // FIXME: chat may be invalid now
        if (!server_is_valid(srv)){
            return;
        }
        if (ret != SRN_OK){
            if (RET_IS_OK(ret)){
                chat_add_misc_message(chat, chat->user->nick, RET_MSG(ret));
            } else {
                chat_add_error_message(chat, chat->user->nick, RET_MSG(ret));
            }
        }
    } else {
        chat_add_sent_message(chat, msg);
    }
}

void server_ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    const char *passwd;
    Server *srv;

    g_return_if_fail(count == 1 || count == 2);
    name = params[0];
    passwd = count == 2 ? params[1] : NULL;

    srv = ctx_get_server(sui);
    g_return_if_fail(server_is_valid(srv));

    sirc_cmd_join(srv->irc, name, passwd);
}

void server_ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 0);
    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(chat);

    if (chat->joined) {
        sirc_cmd_part(srv->irc, chat->name, "Leave.");
    } else {
        server_rm_chat(srv, chat->name);
    }
}

void server_ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    Server *srv;

    g_return_if_fail(count == 1);
    name = params[0];

    srv = ctx_get_server(sui);
    g_return_if_fail(server_is_valid(srv));

    server_add_chat(srv, name);
}

void server_ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(chat);

    server_rm_chat(srv, chat->name);
}

void server_ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(chat);

    sirc_cmd_kick(srv->irc, nick, chat->name, "Kick.");
}

void server_ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(chat);

    sirc_cmd_invite(srv->irc, nick, chat->name);
}

void server_ui_event_whois(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;

    g_return_if_fail(count == 1);
    nick = params[0];

    srv = ctx_get_server(sui);
    g_return_if_fail(server_is_valid(srv));

    sirc_cmd_whois(srv->irc, nick);
}

void server_ui_event_ignore(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    chat = ctx_get_chat(sui);
    g_return_if_fail(chat);

    nick_filter_add_nick(chat, nick);
}

void server_ui_event_cutover(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 0);

    srv = ctx_get_server(sui);
    g_return_if_fail(server_is_valid(srv));
    chat = ctx_get_chat(sui);
    g_return_if_fail(chat);

    srv->cur_chat = chat;
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
