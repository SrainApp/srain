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
 * @file app_irc_event.c
 * @brief Application IRC event callbacks
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-07-19
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glib.h>

#include "core/core.h"
#include "sui/sui.h"
#include "sirc/sirc.h"
#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"
#include "utils.h"

static gboolean irc_period_ping(gpointer user_data);

static void irc_event_connect(SircSession *sirc, const char *event);
static void irc_event_connect_fail(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);
static void irc_event_disconnect(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

static void irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_ctcp_req(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_ctcp_rsp(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);
static void irc_event_cap(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);
static void irc_event_authenticate(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);
static void irc_event_ping(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);
static void irc_event_pong(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);
static void irc_event_error(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);
static void irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char *params[], int count);
static void irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char *params[], int count);

void srn_application_init_irc_event(SrnApplication *app) {
    app->irc_events.connect = irc_event_connect;
    app->irc_events.connect_fail = irc_event_connect_fail;
    app->irc_events.disconnect = irc_event_disconnect;

    app->irc_events.welcome = irc_event_welcome;
    app->irc_events.nick = irc_event_nick;
    app->irc_events.quit = irc_event_quit;
    app->irc_events.join = irc_event_join;
    app->irc_events.part = irc_event_part;
    app->irc_events.mode = irc_event_mode;
    app->irc_events.umode = irc_event_umode;
    app->irc_events.topic = irc_event_topic;
    app->irc_events.kick = irc_event_kick;
    app->irc_events.channel = irc_event_channel;
    app->irc_events.privmsg = irc_event_privmsg;
    app->irc_events.notice = irc_event_notice;
    app->irc_events.channel_notice = irc_event_channel_notice;
    app->irc_events.invite = irc_event_invite;
    app->irc_events.ctcp_req = irc_event_ctcp_req;
    app->irc_events.ctcp_rsp = irc_event_ctcp_rsp;
    app->irc_events.cap = irc_event_cap;
    app->irc_events.authenticate = irc_event_authenticate;
    app->irc_events.ping = irc_event_ping;
    app->irc_events.pong = irc_event_pong;
    app->irc_events.error = irc_event_error;
    app->irc_events.numeric = irc_event_numeric;
}

static void irc_event_connect(SircSession *sirc, const char *event){
    GSList *list;
    SrnRet ret;
    SrnServer *srv;
    SrnChat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    ret = srn_server_state_transfrom(srv, SRN_SERVER_ACTION_CONNECT_FINISH);
    g_return_if_fail(RET_IS_OK(ret));

    /* Default state */
    srv->registered = FALSE;
    srv->loggedin = FALSE;
    srv->negotiated = FALSE;

    srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
            _("Connected to %1$s(%2$s:%3$d)"),
            srv->cfg->name, srv->addr->host, srv->addr->port);
    list = srv->chat_list;
    while (list){
        chat = list->data;
        srn_chat_add_misc_message_fmt(chat, chat->_user,
                _("Connected to %1$s(%2$s:%3$d)"),
                srv->cfg->name, srv->addr->host, srv->addr->port);
        list = g_slist_next(list);
    }

    /* Start client capability negotiation */
    sirc_cmd_cap_ls(srv->irc, "302");

    if (!str_is_empty(srv->cfg->passwd)){
        /* Send connection password, you should send it command before sending
         * the NICK/USER combination. */
        sirc_cmd_pass(srv->irc, srv->cfg->passwd);
    }
    sirc_cmd_nick(srv->irc, srv->user->nick);
    sirc_cmd_user(srv->irc, srv->user->username, "hostname", "servername",
            srv->user->realname);
}

static void irc_event_connect_fail(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    GSList *list;
    const char *msg;
    SrnRet ret;
    SrnServer *srv;

    g_return_if_fail(count == 1);
    msg = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    ret = srn_server_state_transfrom(srv, SRN_SERVER_ACTION_CONNECT_FAIL);
    g_return_if_fail(RET_IS_OK(ret));
    if (!srn_server_is_valid(srv)) {
        /* FIXME: SERVE_STATE_CONNECTING + SRN_SERVER_ACTION_QUIT will trigger a
         * CONNECT_FAIL event and the server will be freed after apply the
         * SRN_SERVER_ACTION_CONNECT_FAIL action.
         */
        return;
    }

    list = srv->chat_list;
    while (list){
        SrnChat *chat;

        chat = list->data;
        srn_chat_add_misc_message_fmt(chat, chat->_user,
                _("Failed to connect to %1$s(%2$s:%3$d): %4$s"),
                srv->cfg->name, srv->addr->host, srv->addr->port, msg);
        if (srv->state == SRN_SERVER_STATE_RECONNECTING){
            srn_chat_add_misc_message_fmt(chat, chat->_user,
                    _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                    srv->cfg->name,
                    srv->addr->host,
                    srv->addr->port,
                    (srv->reconn_interval * 1.0) / 1000);
        }

        list = g_slist_next(list);
    }

    srn_chat_add_error_message_fmt(srv->chat, srv->chat->_user,
            _("Failed to connect to %1$s(%2$s:%3$d): %4$s"),
            srv->cfg->name, srv->addr->host, srv->addr->port, msg);
    /* If user trying connect to a TLS port via non-TLS connection, it will
     * be reset, give user some hints. */
    if (!srv->cfg->irc->tls
            && (srv->addr->port == 6697 || srv->addr->port == 7000)) {
        srn_chat_add_error_message_fmt(srv->chat, srv->chat->_user,
                _("It seems that you connect to a TLS port(%1$d) without enable TLS connection, try to enable it and reconnect"),
                srv->addr->port);
    }
    if (srv->state == SRN_SERVER_STATE_RECONNECTING){
        srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                srv->cfg->name,
                srv->addr->host,
                srv->addr->port,
                (srv->reconn_interval * 1.0) / 1000);
    }
}

static void irc_event_disconnect(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *msg;
    GSList *list;
    SrnRet ret;
    SrnServer *srv;

    g_return_if_fail(count == 1);
    msg = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    if (srv->last_action == SRN_SERVER_ACTION_RECONNECT){
        msg = _("Ping time out");
    }

    /* Stop period ping */
    if (srv->ping_timer){
        DBG_FR("Ping timer %d removed", srv->ping_timer);
        g_source_remove(srv->ping_timer);
        srv->ping_timer = 0;
    }

    ret = srn_server_state_transfrom(srv, SRN_SERVER_ACTION_DISCONNECT_FINISH);
    g_return_if_fail(RET_IS_OK(ret));
    if (!srn_server_is_valid(srv)) {
        /* SRN_SERVER_ACTION_QUIT will often trigger a DISCONNECT event and the
         * server will be freed after apply the SRN_SERVER_ACTION_DISCONNECT_FINISH
         * action.
         */
        return;
    }

    /* Update state */
    srv->registered = FALSE;
    srv->loggedin = FALSE;
    srv->negotiated = FALSE;

    /* Mark all channels as unjoined */
    list = srv->chat_list;
    while (list){
        SrnChat *chat;

        chat = list->data;
        // Mark all chats as unjoined
        srn_chat_set_is_joined(chat, FALSE);
        // Only report error message to server chat
        srn_chat_add_misc_message_fmt(chat, chat->_user,
                _("Disconnected from %1$s(%2$s:%3$d): %4$s"),
                srv->cfg->name, srv->addr->host, srv->addr->port, msg);
        if (srv->state == SRN_SERVER_STATE_RECONNECTING){
            srn_chat_add_misc_message_fmt(chat, chat->_user,
                    _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                    srv->cfg->name,
                    srv->addr->host,
                    srv->addr->port,
                    (srv->reconn_interval * 1.0) / 1000);
        }

        list = g_slist_next(list);
    }

    srn_chat_add_error_message_fmt(srv->chat, srv->chat->_user,
            _("Disconnected from %1$s(%2$s:%3$d): %4$s"),
            srv->cfg->name, srv->addr->host, srv->addr->port, msg);
    if (srv->state == SRN_SERVER_STATE_RECONNECTING){
        srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                srv->cfg->name,
                srv->addr->host,
                srv->addr->port,
                (srv->reconn_interval * 1.0) / 1000);
    }
}

static void irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char **params, int count){
    const char *nick ;
    GSList *list;
    SrnServer *srv;

    g_return_if_fail(count >= 1);
    nick = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    // You have registered when you recived a RPL_WELCOME(001) message
    srv->registered = TRUE;

    /* Start peroid ping */
    srv->last_pong = get_time_since_first_call_ms();
    srv->ping_timer = g_timeout_add(SRN_SERVER_PING_INTERVAL, irc_period_ping, srv);
    DBG_FR("Ping timer %d created", srv->ping_timer);

    // Set your actually nick
    srn_server_rename_user(srv, srv->user, nick);

    /* Join all channels already exists */
    list = srv->chat_list;
    while (list){
        SrnChat *chat = list->data;
        if (sirc_target_is_channel(srv->irc, chat->name)){
            sirc_cmd_join(srv->irc, chat->name, NULL);
        }
        list = g_slist_next(list);
    }
}

static void irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *old_nick;
    const char *new_nick;
    GSList *lst;
    SrnRet ret;
    SrnServer *srv;
    SrnServerUser *srv_user;

    g_return_if_fail(count >= 1);
    old_nick = origin;
    new_nick = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    srv_user = srn_server_get_user(srv, old_nick);
    g_return_if_fail(srv_user);

    lst = srv_user->chat_user_list;
    while (lst){
        SrnChatUser *chat_user;

        chat_user = lst->data;
        // TODO: dialog nick track support
        srn_chat_add_misc_message_fmt(chat_user->chat, chat_user,
                _("%1$s is now known as %2$s"), old_nick, new_nick);
        lst = g_slist_next(lst);
    }
    if (srv_user->is_me){
        srn_chat_add_misc_message_fmt(srv->chat, srv->chat->user,
                _("%1$s is now known as %2$s"), old_nick, new_nick);
    }

    srn_server_rename_user(srv, srv_user, new_nick);
}

static void irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    const char *reason;
    GSList *lst;
    SrnServer *srv;
    SrnServerUser *srv_user;

    g_return_if_fail(count >= 1);
    reason = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    srv_user = srn_server_get_user(srv, origin);
    g_return_if_fail(srv_user);

    if (reason) {
        snprintf(buf, sizeof(buf), _("%1$s has quit: %2$s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%1$s has quit"), origin);
    }

    lst = srv_user->chat_user_list;
    while (lst){
        SrnChatUser *chat_user;

        // TODO: dialog support
        chat_user = lst->data;
        srn_chat_add_misc_message(chat_user->chat, chat_user, buf);
        lst = g_slist_next(lst);
    }

    srn_server_user_set_is_online(srv_user, FALSE);

    /* You quit */
    if (srv_user->is_me){
        srn_server_state_transfrom(srv, SRN_SERVER_ACTION_QUIT);
    }
}

static void irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    const char *chan;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 1);
    chan = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    srv_user = srn_server_add_and_get_user(srv, origin);
    srn_server_user_set_is_online(srv_user, TRUE);
    if (srv_user->is_me) {
        /* You has join a channel */
        srn_server_add_chat(srv, chan);
    }

    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);

    if (srv_user->is_me) {
        snprintf(buf, sizeof(buf), _("You have joined"));
        srn_chat_set_is_joined(chat, TRUE);
        chat_user = chat->user;
    } else {
        snprintf(buf, sizeof(buf), _("%1$s has joined"), origin);
        chat_user = srn_chat_add_and_get_user(chat, srv_user);
    }

    g_return_if_fail(!chat_user->is_joined);
    srn_chat_user_set_is_joined(chat_user, TRUE);

    srn_chat_add_misc_message(chat, chat_user, buf);
}

static void irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    const char *chan;
    const char *reason;
    SrnServer *srv;
    SrnChat *chat;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 1);
    chan = params[0];
    reason = count == 2 ? params[1] : NULL;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);
    chat_user = srn_chat_get_user(chat, origin);
    g_return_if_fail(chat_user);
    g_return_if_fail(chat_user->is_joined);

    if (reason){
        snprintf(buf, sizeof(buf), _("%1$s has left: %2$s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%1$s has left"), origin);
    }

    srn_chat_add_misc_message(chat, chat_user, buf);
    srn_chat_user_set_is_joined(chat_user, FALSE);

    /* You has left a channel */
    if (chat_user->srv_user->is_me){
        srn_chat_set_is_joined(chat, FALSE);
        srn_server_rm_chat(srv, chat);
    }
}

static void irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *chan;
    GString *buf;
    GString *modes;
    SrnServer *srv;
    SrnChat *chat;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 1);
    chan = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);
    chat_user = srn_chat_get_user(chat, origin);
    g_return_if_fail(chat_user);

    modes = g_string_new(NULL);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(modes, "%s ", params[i]);
    }
    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %1$s %2$s by %3$s"), chan, modes->str, origin);

    srn_chat_add_misc_message(chat, chat_user, buf->str);

    g_string_free(modes, TRUE);
    g_string_free(buf, TRUE);

    // FIXME: Only process the first mode
    do {
        const char *mode;
        const char *mode_args;
        SrnServerUser *srv_user;
        SrnChatUser *chat_user;
        SrnChatUserType type;

        // <channel> *( ( "-" / "+" ) *<modes> *<modeparams>
        g_return_if_fail(count >= 3);
        mode = params[1];
        g_return_if_fail(strlen(mode)>=2);
        mode_args = params[2];

        srv_user = srn_server_add_and_get_user(srv, mode_args);
        g_return_if_fail(srv_user);
        chat_user = srn_chat_add_and_get_user(chat, srv_user);
        g_return_if_fail(chat_user);

        type = SRN_SERVER_USER_CHIGUA;
        if (mode[0] == '-'){
            type = SRN_SERVER_USER_CHIGUA;
        } else if (mode[0] == '+'){
            switch (mode[1]){
                case 'q':
                    type = SRN_SERVER_USER_OWNER;
                    break;
                case 'a':
                    type = SRN_SERVER_USER_ADMIN;
                    break;
                case 'o':
                    type = SRN_SERVER_USER_FULL_OP;
                    break;
                case 'h':
                    type = SRN_SERVER_USER_HALF_OP;
                    break;
                case 'v':
                    type = SRN_SERVER_USER_VOICED;
                    break;
                default:
                    break;
            }
        } else {
            ERR_FR("Unrecognized mode: %s. chan: %s, mode_args: %s",
                    mode, chan, mode_args);
        }
        srn_chat_user_set_type(chat_user, type);
    } while (0);
}

static void irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *nick;
    GString *buf;
    GString *modes;
    SrnServer *srv;

    g_return_if_fail(count >= 1);
    nick = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    modes = g_string_new(NULL);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(modes, "%s ", params[i]);
    }
    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %1$s %2$s by %3$s"), nick, modes->str, origin);

    // FIXME: use srv->chat->user for now
    srn_chat_add_misc_message(srv->chat, srv->chat->user, buf->str);

    g_string_free(modes, TRUE);
    g_string_free(buf, TRUE);
}

static void irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *chan;
    const char *topic;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 2);
    chan = params[0];
    topic = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    chat_user = srn_chat_add_and_get_user(chat, srv_user);
    g_return_if_fail(chat_user);

    srn_chat_set_topic(chat, chat_user, topic);

    if (strlen(topic) == 0) {
        srn_chat_add_misc_message_fmt(chat, chat_user, _("%1$s cleared topic"),
                origin, topic);
    } else {
        srn_chat_add_misc_message_fmt(chat, chat_user,
                _("%1$s changed topic to:\n\t%2$s"), origin, topic);
    }
}

static void irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    const char *chan;
    const char *kicked;
    const char *reason;
    SrnServer *srv;
    SrnChat *chat;
    SrnChatUser *kick_chat_user;
    SrnChatUser *kicked_chat_user;

    g_return_if_fail(count >= 2);
    chan = params[0];
    kicked = params[1];
    reason = count == 3 ? params[2] : NULL;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);
    kick_chat_user = srn_chat_get_user(chat, origin);
    g_return_if_fail(kick_chat_user);
    kicked_chat_user = srn_chat_get_user(chat, kicked);
    g_return_if_fail(kicked_chat_user);

    /* You are kicked 23333 */
    if (kicked_chat_user->srv_user->is_me){
        if (reason){
            snprintf(buf, sizeof(buf), _("You are kicked by %1$s: %2$s"),
                    origin, reason);
        } else {
            snprintf(buf, sizeof(buf), _("You are kicked by %1$s"),
                    origin);
        }
        srn_chat_set_is_joined(chat, FALSE);
    } else {
        if (reason){
            snprintf(buf, sizeof(buf), _("%1$s are kicked by %2$s: %3$s"),
                    kicked, origin, reason);
        } else {
            snprintf(buf, sizeof(buf), _("%1$s are kicked by %2$s"),
                    kicked, origin);
        }
    }

    srn_chat_user_set_is_joined(kicked_chat_user, FALSE);
    srn_chat_add_error_message(chat, kick_chat_user, buf);
}

static void irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *chan;
    const char *msg;
    SrnServer *srv;
    SrnChat *chat;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 2);
    chan = params[0];
    msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);
    chat_user = srn_chat_get_user(chat, origin);
    g_return_if_fail(chat_user);

    srn_chat_add_recv_message(chat, chat_user, msg);
}

static void irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *msg;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 2);
    msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    if (sirc_target_is_servername(sirc, origin)
            || sirc_target_is_service(sirc, origin)){
        chat = srv->chat;
    } else {
        chat = srn_server_add_and_get_chat(srv, origin);
    }
    g_return_if_fail(chat);
    chat_user = srn_chat_add_and_get_user(chat, srv_user);
    g_return_if_fail(chat_user);

    srn_chat_add_recv_message(chat, chat_user, msg);
}

static void irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *msg;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 2);
    msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    if (sirc_target_is_servername(sirc, origin)
            || sirc_target_is_service(sirc, origin)){
        chat = srv->chat;
    } else {
        chat = srn_server_add_and_get_chat(srv, origin);
    }
    g_return_if_fail(chat);
    chat_user = srn_chat_add_and_get_user(chat, srv_user);
    g_return_if_fail(chat_user);

    srn_chat_add_notice_message(chat, chat_user, msg);
}

static void irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *chan;
    const char *msg;
    SrnServer *srv;
    SrnChat *chat;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 2);
    chan = params[0];
    msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat(srv, chan);
    g_return_if_fail(chat);
    chat_user = srn_chat_get_user(chat, origin);
    g_return_if_fail(chat_user);

    srn_chat_add_notice_message(chat, chat_user, msg);
}

static void irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *nick;
    const char *chan;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 2);
    nick = params[0];
    chan = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    chat = srn_server_add_and_get_chat(srv, origin);
    g_return_if_fail(chat);
    chat_user = srn_chat_add_and_get_user(chat, srv_user);
    g_return_if_fail(chat_user);

    if (!sirc_target_equal(srv->user->nick, nick)){
        WARN_FR("Received a invite message to %s", nick);
        g_return_if_reached();
    }

    srn_chat_add_misc_message_fmt(chat, chat_user,
            _("%1$s invites you into %2$s"), origin, chan);
}

static void irc_event_ctcp_req(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *target;
    const char *msg;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    g_return_if_fail(count >= 1);
    target = params[0];
    msg = count == 2 ? params[1] : NULL;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    if (sirc_target_is_channel(sirc, target)){
        chat = srn_server_get_chat(srv, target);
    } else {
        if (strcmp(event, "ACTION") == 0) {
            // Only create chat for ACTION message
            chat = srn_server_add_and_get_chat(srv, origin);
        } else {
            chat = srn_server_get_chat_fallback(srv, origin);
        }
    }
    g_return_if_fail(chat);
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    chat_user = srn_chat_add_and_get_user(chat, srv_user);
    g_return_if_fail(chat_user);

    /* CTCP response, according to https://modern.ircdocs.horse/ctcp.html */
    if (strcmp(event, "ACTION") == 0) {
        // Nothing to do with ACTION message
    } else if (strcmp(event, "CLIENTINFO") == 0) {
        sirc_cmd_ctcp_rsp(srv->irc, origin, event, "CLIENTINFO");
    } else if (strcmp(event, "DCC") == 0) {
        // TODO
    } else if (strcmp(event, "FINGER") == 0) {
        sirc_cmd_ctcp_rsp(srv->irc, origin, event,
                PACKAGE_NAME " " PACKAGE_VERSION PACKAGE_BUILD);
    } else if (strcmp(event, "PING") == 0) {
        sirc_cmd_ctcp_rsp(srv->irc, origin, event, msg);
    } else if (strcmp(event, "SOURCE") == 0) {
        sirc_cmd_ctcp_rsp(srv->irc, origin, event, PACKAGE_WEBSITE);
    } else if (strcmp(event, "TIME") == 0) {
        GTimeVal val;
        g_get_current_time(&val);
        // ISO 8601 is recommend
        char *time = g_time_val_to_iso8601(&val);
        if (time){
            sirc_cmd_ctcp_rsp(srv->irc, origin, event, time);
            g_free(time);
        }
    } else if (strcmp(event, "VERSION") == 0) {
        sirc_cmd_ctcp_rsp(srv->irc, origin, event,
                PACKAGE_NAME " " PACKAGE_VERSION PACKAGE_BUILD);
    } else if (strcmp(event, "USERINFO") == 0) {
        sirc_cmd_ctcp_rsp(srv->irc, origin, event, srv->user->realname);
    } else {
        WARN_FR("Unknown CTCP message: %s", event);
    }

    /* Show it on ui */
    if (strcmp(event, "ACTION") == 0) {
        srn_chat_add_action_message(chat, chat_user, msg);
    } else if (strcmp(event, "DCC") == 0) {
        srn_chat_add_error_message_fmt(chat, chat_user,
                _("Received unsupported CTCP %1$s request form %2$s"),
                event, origin);
    } else {
        srn_chat_add_misc_message_fmt(chat, chat_user,
                _("Received CTCP %1$s request form %2$s"), event, origin);
    }
}

static void irc_event_ctcp_rsp(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *target;
    const char *msg;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    // TODO: ignore ignored user's CTCP request

    g_return_if_fail(count >= 2);
    target = params[0];
    msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    chat = srn_server_get_chat_fallback(srv, origin);
    g_return_if_fail(chat);
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    chat_user = srn_chat_add_and_get_user(chat, srv_user);
    g_return_if_fail(chat_user);

    if (strcmp(event, "ACTION") == 0) {
        // Is there any ACTION response?
    } else if (strcmp(event, "CLIENTINFO") == 0
            || strcmp(event, "FINGER") == 0
            || strcmp(event, "SOURCE") == 0
            || strcmp(event, "TIME") == 0
            || strcmp(event, "VERSION") == 0
            || strcmp(event, "USERINFO") == 0){
        srn_chat_add_misc_message_fmt(chat, chat_user,
                _("Received CTCP %1$s response from %2$s: %3$s"),
                event, origin, msg);
    } else if (strcmp(event, "DCC") == 0) {
        // TODO
        srn_chat_add_error_message_fmt(chat, chat_user,
                _("Received unsupported CTCP %1$s response form %2$s"),
                event, origin);
    } else if (strcmp(event, "PING") == 0) {
        unsigned long time;
        unsigned long nowtime;

        time = strtoul(msg, NULL, 10);
        nowtime = get_time_since_first_call_ms();

        if (time != 0 && nowtime >= time){
            /* Update dalay and pong time */
            srn_chat_add_misc_message_fmt(chat, chat_user,
                    _("Latency between %1$s: %2$lums"), origin, nowtime - time);
        } else {
        srn_chat_add_misc_message_fmt(chat, chat_user,
                _("Received CTCP %1$s response from %2$s: %3$s"), event, origin, msg);
        }
    } else {
        WARN_FR("Unknown CTCP message: %s", event);
    }
}

static void irc_event_cap(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count){
    bool multiline;
    bool cap_end;
    const char *cap_event;
    const char *rawcaps;
    char **caps;
    SrnServer *srv;

    g_return_if_fail(count >= 3);
    cap_end = FALSE;
    cap_event = params[1];
    rawcaps = params[2];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    /* Multi line replies */
    multiline = ((g_ascii_strcasecmp(cap_event, "LS") == 0
                || g_ascii_strcasecmp(cap_event, "LIST") == 0)
            && g_ascii_strcasecmp(rawcaps, "*") == 0);
    if (multiline){
        g_return_if_fail(count == 4);
        rawcaps = params[3];
    }
    caps = g_strsplit(rawcaps, " ", 0);

    /* Process CAP event */
    if (g_ascii_strcasecmp(cap_event, "LS") == 0){
        GString *buf;

        buf = g_string_new(NULL);
        for (int i = 0; caps[i]; i++){
            const char *name;
            char *value;

            name = caps[i];
            value = strchr(caps[i], '='); // <name>[=<value>]
            if (value) {
                *value = '\0';
                value++; // Skip '='
            }
            if (srn_server_cap_is_support(srv->cap, name, value)
                    && RET_IS_OK(srn_server_cap_server_enable(srv->cap, name, TRUE))){
                g_string_append_printf(buf, "%s ", name);
            }
        }

        srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                _("Server capabilities: %1$s"), rawcaps);
        if (buf->len > 0){
            srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                    _("Requesting capabilities: %1$s"), buf->str);
            sirc_cmd_cap_req(sirc, buf->str);
        } else {
            srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                    _("No capability to be requested"));
            cap_end = TRUE; // It's time to end the negotiation
        }

        g_string_free(buf, TRUE);
    } else if (g_ascii_strcasecmp(cap_event, "NEW") == 0){
        GString *buf;

        buf = g_string_new(NULL);
        for (int i = 0; caps[i]; i++){
            const char *name;
            char *value;

            name = caps[i];
            value = strchr(caps[i], '='); // <name>[=<value>]
            if (value) {
                *value = '\0';
                value++; // Skip '='
            }
            if (srn_server_cap_is_support(srv->cap, name, value)
                    && RET_IS_OK(srn_server_cap_server_enable(srv->cap, name, TRUE))){
                g_string_append_printf(buf, "%s ", name);
            }
        }

        srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                _("Server has new capabilities: %1$s"), rawcaps);
        if (buf->len > 0){
            srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                    _("Requesting new capabilities: %1$s"), buf->str);
            sirc_cmd_cap_req(sirc, buf->str);
        } else {
            srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                    _("No new capability to be requested"));
        }

        g_string_free(buf, TRUE);
    } else if (g_ascii_strcasecmp(cap_event, "LIST") == 0){
        if (strlen(rawcaps) > 0){
            srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                    _("Acknowledged capabilities: %1$s"), rawcaps);
        } else {
            srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                    _("No acknowledged capability"));
        }
    } else if (g_ascii_strcasecmp(cap_event, "ACK") == 0){
        for (int i = 0; caps[i]; i++){
            bool enable;
            const char *name;

            name = caps[i];
            switch (name[0]) {
                case '-': // Disable modifier
                    enable = FALSE;
                    name++;
                    break;
                case '~': // ACK modifier
                case '=': // Sticky modifier
                    // TODO: deprecated, skip for now
                    name++;
                    // Fallthrough
                default:
                    enable = TRUE;
            }

            if (!RET_IS_OK(srn_server_cap_client_enable(srv->cap, name, enable))){
                WARN_FR("Unknown capability: %s", name);
            }

            if (srn_server_cap_all_enabled(srv->cap)){
                cap_end = TRUE;
            }
        }

    } else if (g_ascii_strcasecmp(cap_event, "DEL") == 0){
        for (int i = 0; caps[i]; i++){
            bool enable;
            const char *name;

            name = caps[i];
            switch (name[0]) {
                case '-': // Disable modifier
                case '~': // ACK modifier
                case '=': // Sticky modifier
                    // TODO: deprecated, skip for now
                    name++;
                    // Fallthrough
                default:
                    enable = FALSE;
            }

            if (!RET_IS_OK(srn_server_cap_client_enable(srv->cap, name, enable))){
                WARN_FR("Unknown capability: %s", name);
            }
        }

        srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                _("Server has deleted capabilities: %1$s"), rawcaps);
    } else if (g_ascii_strcasecmp(cap_event, "NAK") == 0){
        cap_end = TRUE;
    } else {
        g_warn_if_reached();
    }

    /* Whether to end the negotiation? */
    if (!srv->negotiated && cap_end){
        sirc_cmd_cap_list(sirc);

        if (srv->cfg->login_method == SRN_LOGIN_METHOD_SASL_PLAIN){
            if (srv->cap->client_enabled.sasl){
                // Negotiation should end after sasl authentication end
            } else {
                srn_chat_add_error_message_fmt(srv->chat, srv->chat->_user,
                        _("SASL authentication is not supported on this server, login skipped"));
                sirc_cmd_cap_end(sirc); // End negotiation
                srv->negotiated = TRUE;
            }
        } else {
            sirc_cmd_cap_end(sirc); // End negotiation
            srv->negotiated = TRUE;
        }
    }

    g_strfreev(caps);
}

static void irc_event_authenticate(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    SrnServer *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    switch (srv->cfg->login_method){
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            {
                char *base64;
                char *login_method;
                GString *str;

                g_return_if_fail(!str_is_empty(srv->cfg->username));
                g_return_if_fail(!str_is_empty(srv->cfg->user_passwd));

                /* ref: https://ircv3.net/specs/extensions/sasl-3.1.html */
                str = g_string_new(NULL);
                str = g_string_append(str, srv->cfg->username);
                str = g_string_append_unichar(str, g_utf8_get_char("\0")); // Unicode null char
                str = g_string_append(str, srv->cfg->username);
                str = g_string_append_unichar(str, g_utf8_get_char("\0")); // Unicode null char
                str = g_string_append(str, srv->cfg->user_passwd);

                // TODO: 400 bytes limit
                base64 = g_base64_encode((const guchar *)str->str, str->len);
                sirc_cmd_authenticate(sirc, base64);

                login_method = srn_login_method_to_string(srv->cfg->login_method);
                srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                        _("Logging in with %1$s as %2$s..."),
                        login_method, srv->cfg->username);

                g_free(login_method);
                g_free(base64);
                g_string_free(str, TRUE);
                break;
            }
        default:
            g_warn_if_reached();
    }
}

static void irc_event_ping(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    // Nothing to do for now
}

static void irc_event_pong(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *msg;
    unsigned long time;
    unsigned long nowtime;
    SrnServer *srv;

    g_return_if_fail(count >= 1);
    msg = params[count - 1]; // time should be the last parameter

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    time = strtoul(msg, NULL, 10);
    nowtime = get_time_since_first_call_ms();

    if (time != 0 && nowtime >= time){
        /* Update dalay and pong time */
        srv->delay = nowtime - time;
        srv->last_pong = nowtime;
        DBG_FR("Delay: %lu ms", srv->delay);
    } else {
        ERR_FR("Wrong timestamp: %s", msg);
    }
}

static void irc_event_error(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    const char *msg;
    SrnServer *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));

    g_return_if_fail(count >= 1);
    msg = params[0];

    srn_chat_add_error_message_fmt(srv->cur_chat, srv->chat->_user,
            _("ERROR: %1$s"), msg);
}

static void irc_event_numeric(SircSession *sirc, int event,
        const char *origin, const char **params, int count){
    SrnServer *srv;
    SrnServerUser *srv_user;
    SrnChatUser *chat_user;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srn_server_is_valid(srv));
    srv_user = srn_server_add_and_get_user(srv, origin);
    g_return_if_fail(srv_user);
    chat_user = srn_chat_add_and_get_user(srv->chat, srv_user);
    g_return_if_fail(chat_user);

    switch (event) {
        case SIRC_RFC_RPL_WELCOME:
        case SIRC_RFC_RPL_YOURHOST:
        case SIRC_RFC_RPL_CREATED:
        case SIRC_RFC_RPL_MOTDSTART:
        case SIRC_RFC_RPL_MOTD:
        case SIRC_RFC_RPL_ENDOFMOTD:
        case SIRC_RFC_RPL_MYINFO:
        case SIRC_RFC_RPL_BOUNCE:
        case SIRC_RFC_RPL_LUSEROP:
        case SIRC_RFC_RPL_LUSERUNKNOWN:
        case SIRC_RFC_RPL_LUSERCHANNELS:
        case SIRC_RFC_RPL_LUSERCLIENT:
        case SIRC_RFC_RPL_LUSERME:
        case SIRC_RFC_RPL_ADMINME:
        case SIRC_RFC_RPL_STATSDLINE:
        case SIRC_RFC_RPL_LOCALUSERS:
        case SIRC_RFC_RPL_GLOBALUSERS:
            {
                GString *buf;

                buf = g_string_new(NULL);
                for (int i = 1; i < count; i++){
                    g_string_append_printf(buf, "%s ", params[i]);
                }
                srn_chat_add_recv_message(srv->chat, chat_user, buf->str);
                g_string_free(buf, TRUE);
                break;
            }
        case SIRC_RFC_ERR_NICKNAMEINUSE:
            {
                const char *nick;

                g_return_if_fail(count >= 3);
                nick = params[1];

                /* If you don't have a nickname (unregistered) yet, try a nick
                 * with a trailing underline('_') */
                if (!srv->registered){
                    char *new_nick = g_strdup_printf("%s_", nick);

                    // FIXME: ircd-seven will truncate the nick without
                    // returning a error message if it reaches the length
                    // limiation, at this time the new_nick is same to the
                    // registered old nick in the server view.
                    sirc_cmd_nick(srv->irc, new_nick);
                    srn_chat_add_misc_message_fmt(srv->chat, srv->chat->_user,
                            _("Trying nickname: \"%1$s\"..."), new_nick);

                    g_free(new_nick);
                }
                goto ERRMSG;
            }

            /************************ NAMES message ************************/
        case SIRC_RFC_RPL_NAMREPLY:
            {
                char *nickptr;
                char *dup_names;
                const char *chan;
                const char *names;
                SrnChat *chat;
                SrnServerUser *srv_user;
                SrnChatUser *chat_user;
                SrnChatUserType type;

                g_return_if_fail(count >= 4);
                chan = params[2];
                names = params[3];

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);

                dup_names = g_strdup(names);
                nickptr = strtok(dup_names, " ");
                for (; nickptr = strtok(NULL, " "); nickptr){
                    switch (nickptr[0]){
                        case '~':
                            nickptr++;
                            type = SRN_SERVER_USER_OWNER;
                            break;
                        case '&':
                            nickptr++;
                            type = SRN_SERVER_USER_ADMIN;
                            break;
                        case '@':
                            nickptr++;
                            type = SRN_SERVER_USER_FULL_OP;
                            break;
                        case '%':
                            nickptr++;
                            type = SRN_SERVER_USER_HALF_OP;
                            break;
                        case '+':
                            nickptr++;
                            type = SRN_SERVER_USER_VOICED;
                            break;
                        default:
                            type = SRN_SERVER_USER_CHIGUA;
                    }
                    srv_user = srn_server_add_and_get_user(srv, nickptr);
                    g_warn_if_fail(srv_user);
                    if (!srv_user) continue;
                    srn_server_user_set_is_online(srv_user, TRUE);

                    chat_user = srn_chat_add_and_get_user(chat, srv_user);
                    g_warn_if_fail(chat_user);
                    if (!chat_user) continue;
                    srn_chat_user_set_is_joined(chat_user, TRUE);
                    srn_chat_user_set_type(chat_user, type);
                }
                g_free(dup_names);
                break;
            }
        case SIRC_RFC_RPL_ENDOFNAMES:
            {
                break;
            }
        case SIRC_RFC_RPL_NOTOPIC:
            {
                const char *chan;
                SrnChat *chat;

                g_return_if_fail(count >= 2);
                chan = params[1];

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);

                srn_chat_add_misc_message_fmt(chat, chat->_user,
                        _("No topic is set"), chan);
                break;
            }
        case SIRC_RFC_RPL_TOPIC:
            {
                const char *chan;
                const char *topic;
                SrnChat *chat;
                SrnChatUser *chat_user;

                g_return_if_fail(count >= 3);
                chan = params[1];
                topic = params[2];

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);
                chat_user = srn_chat_add_and_get_user(chat, srv_user);
                g_return_if_fail(chat_user);

                srn_chat_set_topic(chat, chat_user, topic);
                srn_chat_add_misc_message_fmt(chat, chat_user,
                        _("The topic of this channel is:\n\t%1$s"), topic);
                break;
            }
        case SIRC_RFC_RPL_TOPICWHOTIME:
            {
                time_t time;
                char timestr[64];
                char *setter;
                const char *chan;
                const char *who;
                SrnChat *chat;

                g_return_if_fail(count >= 4);
                chan = params[1];
                who = params[2];
                time = strtoul(params[3], NULL, 10);

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);

                time_to_str(time, timestr, sizeof(timestr), _("%Y-%m-%d %T"));
                setter = g_strdup_printf(_("By %1$s at %2$s"), who, timestr);
                srn_chat_set_topic_setter(chat, setter);
                g_free(setter);
                break;
            }

            /************************ WHOIS message ************************/
        case SIRC_RFC_RPL_WHOISUSER:
            {
                const char *nickname;
                const char *username;
                const char *hostname;
                const char *realname;

                g_return_if_fail(count >= 6);
                nickname = params[1];
                username = params[2];
                hostname = params[3];
                realname = params[4];

                // TODO: dont show WHOIS message in message list
                // srn_chat_add_misc_message_fmt(srv->cur_chat, user, "%s <%s@%s> %s",
                // nickname, username, hostname, realname);
                break;
            }
        case SIRC_RFC_RPL_WHOISCHANNELS:
            {
                const char *msg;

                g_return_if_fail(count >= 3);
                msg = params[2];

                // srn_chat_add_misc_message_fmt(srv->cur_chat, user, _("%1$s is member of %2$s"), params[1], msg);
                break;
            }
        case SIRC_RFC_RPL_WHOISSERVER:
            {
                const char *msg;

                g_return_if_fail(count >= 4);
                msg = params[3];

                // srn_chat_add_misc_message_fmt(srv->cur_chat, user, _("%1$s is attached to %2$s at \"%3$s\""),
                // params[1], params[2], msg);
                break;
            }
        case SIRC_RFC_RPL_WHOISIDLE:
            {
                char timestr[64];
                time_t since;
                const char *who;
                const char *sec;

                g_return_if_fail(count >= 4);
                who = params[1];
                sec = params[2];
                since = strtoul(params[3], NULL, 10);

                // time_to_str(since, timestr, sizeof(timestr), _("%Y-%m-%d %T"));
                // srn_chat_add_misc_message_fmt(srv->cur_chat, user, _("%1$s is idle for %2$s seconds since %3$s"),
                        // who, sec, timestr);
                break;
            }
        case SIRC_RFC_RPL_WHOWAS_TIME:
            {
                const char *msg;

                g_return_if_fail(count >= 4);
                msg = params[3];

                // srn_chat_add_misc_message_fmt(srv->cur_chat, user, _("%1$s %2$s %3$s"),
                        // params[1], msg, params[2]);
                break;
            }
        case SIRC_RFC_RPL_WHOISHOST:
        case SIRC_RFC_RPL_WHOISSECURE:
            {
                const char *msg;

                g_return_if_fail(count >= 3);
                msg = params[2];

                // srn_chat_add_misc_message_fmt(srv->cur_chat, user, _("%1$s %2$s"), params[1], msg);
                break;
            }
        case SIRC_RFC_RPL_ENDOFWHOIS:
            {
                const char *msg;

                g_return_if_fail(count >= 3);
                msg = params[2];

                // srn_chat_add_misc_message(srv->cur_chat, user, msg);
                break;
            }

            /************************ WHO message ************************/
        case SIRC_RFC_RPL_WHOREPLY:
            {
                const char *nick;
                const char *realname;

                g_return_if_fail(count >= 7);
                nick = params[5];
                // params[count - 1] = "<hopcount> <realname>", Skip ' '
                realname = strchr(params[6], ' ');

                // TODO
                break;
            }
        case SIRC_RFC_RPL_ENDOFWHO:
            {
                break;
            }
            /************************ BANLIST message ************************/
        case SIRC_RFC_RPL_BANLIST:
            {
                const char *chan;
                const char *banmask;
                SrnChat *chat;
                SrnChatUser *chat_user;

                g_return_if_fail(count >= 3);
                chan = params[1];
                banmask = params[2];

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);
                chat_user = srn_chat_add_and_get_user(chat, srv_user);
                g_return_if_fail(chat_user);

                srn_chat_add_misc_message_fmt(chat, chat_user,
                        _("%1$s: %2$s"), chan, banmask);
                // TODO: <time_left> and <reason> are not defined in RFC
                break;
            }
        case SIRC_RFC_RPL_ENDOFBANLIST:
            {
                const char *chan;
                const char *msg;
                SrnChat *chat;
                SrnChatUser *chat_user;

                g_return_if_fail(count >= 3);
                chan = params[1];
                msg = params[2];

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);
                chat_user = srn_chat_add_and_get_user(chat, srv_user);
                g_return_if_fail(chat_user);

                srn_chat_add_misc_message_fmt(chat, chat_user, _("%1$s: %2$s"),
                        chan, msg);
                break;
            }
            /************************ LIST message ************************/
        case SIRC_RFC_RPL_LISTSTART:
            {
                g_return_if_fail(count >= 1);

                sui_chan_list_start(srv->chat->ui);
                break;
            }
        case SIRC_RFC_RPL_LIST:
            {
                int users;
                const char *chan;
                const char *topic;

                g_return_if_fail(count >= 4);
                chan = params[1];
                users = atoi(params[2]);
                topic = params[3];

                sui_chan_list_add(srv->chat->ui, chan, users, topic);
                break;
            }
        case SIRC_RFC_RPL_LISTEND:
            {
                g_return_if_fail(count >= 1);

                sui_chan_list_end(srv->chat->ui);
                break;
            }
            /************************ SASL message ************************/
        case SIRC_RFC_RPL_LOGGEDIN:
            {
                const char *msg;

                g_return_if_fail(count >= 4);
                msg = params[3];

                srv->loggedin = TRUE;
                sirc_cmd_cap_end(sirc); // End negotiation
                srn_chat_add_recv_message(srv->chat, chat_user, msg);
                break;
            }
        case SIRC_RFC_RPL_SASLSUCCESS:
            {
                const char *msg;

                g_return_if_fail(count >= 2);
                msg = params[1];

                srn_chat_add_recv_message(srv->chat, chat_user, msg);
                break;
            }
        case SIRC_RFC_RPL_LOGGEDOUT:
            {
                const char *msg;

                g_return_if_fail(count >= 3);
                msg = params[2];

                srv->loggedin = FALSE;
                srn_chat_add_recv_message(srv->chat, chat_user, msg);
                break;
            }
        case SIRC_RFC_ERR_NICKLOCKED:
        case SIRC_RFC_ERR_SASLFAIL:
        case SIRC_RFC_ERR_SASLTOOLONG:
        case SIRC_RFC_RPL_SASLMECHS:
            {
                sirc_cmd_authenticate(sirc, "*"); // Abort the authentication
                goto ERRMSG;
            }
        case SIRC_RFC_ERR_SASLABORTED:
            {
                sirc_cmd_cap_end(sirc); // End negotiation
                goto ERRMSG;
            }
        case SIRC_RFC_ERR_SASLALREADY:
            {
                goto ERRMSG;
            }
            /************************ MISC message ************************/
        case SIRC_RFC_RPL_CHANNEL_URL:
            {
                const char *chan;
                const char *msg;
                SrnChat *chat;
                SrnChatUser *chat_user;

                g_return_if_fail(count >= 3);
                chan = params[1];
                msg = params[2];

                chat = srn_server_get_chat(srv, chan);
                g_return_if_fail(chat);
                chat_user = srn_chat_add_and_get_user(chat, srv_user);
                g_return_if_fail(chat_user);

                srn_chat_add_misc_message_fmt(chat, chat_user,
                        _("URL of %1$s is: %2$s"), chan, msg);
                break;
            }
        default:
            {
                // Error message
                if (event >= 400 && event < 600){
                    goto ERRMSG;
                }

                // Unknown message
                {
                    GString *buf;

                    buf = g_string_new(NULL);
                    for (int i = 0; i < count; i++){
                        buf = g_string_append(buf, params[count-1]); // reason
                        if (i != count - 1){
                            buf = g_string_append(buf, ", ");
                        }
                    }

                    WARN_FR("Unspported message, You can report it at " PACKAGE_WEBSITE);
                    WARN_FR("server: %s, event: %d, origin: %s, count: %u, params: [%s]",
                            srv->cfg->name, event, origin, count, buf->str);

                    g_string_free(buf, TRUE);
                }
                return;
ERRMSG:
                /* Add error message to UI, usually the params of error message
                 * is ["<nick>", ... "<reason>"] */
                {
                    GString *buf;

                    buf = g_string_new(NULL);

                    for (int i = 1; i < count - 1; i++){ // skip nick
                        buf = g_string_append(buf, params[i]);
                        buf = g_string_append_c(buf, ' ');
                        if (i == count - 2){
                            buf = g_string_append_c(buf, ':');
                        }
                    }
                    if (count >= 2){
                        buf = g_string_append(buf, params[count-1]); // reason
                    }

                    srn_chat_add_error_message_fmt(srv->cur_chat, srv->cur_chat->_user,
                            _("ERROR[%1$3d] %2$s"), event, buf->str);

                    g_string_free(buf, TRUE);
                }
                return;
            }
    }
}

static gboolean irc_period_ping(gpointer user_data){
    char timestr[64];
    unsigned long time;
    SrnServer *srv;

    srv = user_data;
    time = get_time_since_first_call_ms();
    snprintf(timestr, sizeof(timestr), "%lu", time);

    DBG_FR("Server %s, %lu ms since last pong, time out: %d ms",
            srv->cfg->name, time - srv->last_pong, SRN_SERVER_PING_TIMEOUT);

    /* Check whether ping time out */
    if (time - srv->last_pong > SRN_SERVER_PING_TIMEOUT){
        WARN_FR("Server %s ping time out, %lums", srv->cfg->name, time - srv->last_pong);

        srv->ping_timer = 0;
        srn_server_state_transfrom(srv, SRN_SERVER_ACTION_RECONNECT);

        return G_SOURCE_REMOVE;
    }

    sirc_cmd_ping(srv->irc, timestr);

    return G_SOURCE_CONTINUE;
}
