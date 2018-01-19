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
 * @file server_irc_event.c
 * @brief Server IRC event callbacks
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-07-19
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glib.h>

#include "server.h"
#include "server_irc_event.h"

#include "sui/sui.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "filter.h"
#include "meta.h"
#include "chat_log.h"
#include "plugin.h"
#include "decorator.h"
#include "utils.h"

static gboolean irc_period_ping(gpointer user_data){
    char timestr[64];
    unsigned long time;
    Server *srv;

    srv = user_data;
    g_return_val_if_fail(server_is_valid(srv), G_SOURCE_REMOVE);

    time = get_time_since_first_call_ms();
    snprintf(timestr, sizeof(timestr), "%lu", time);

    DBG_FR("%lu ms since last pong, time out: %d ms",
            time - srv->last_pong, SERVER_PING_TIMEOUT);

    /* Check whether ping time out */
    if (time - srv->last_pong > SERVER_PING_TIMEOUT){
        WARN_FR("Server %s ping timeout, %lums", srv->prefs->name, time - srv->last_pong);

        srv->ping_timer = 0;
        server_disconnect(srv, SERVER_DISCONN_REASON_TIMEOUT);

        return G_SOURCE_REMOVE;
    }

    sirc_cmd_ping(srv->irc, timestr);

    return G_SOURCE_CONTINUE;
}

static gboolean irc_reconnect(gpointer user_data){
    Server *srv;

    srv = user_data;
    g_return_val_if_fail(server_is_valid(srv), G_SOURCE_REMOVE);
    g_return_val_if_fail(srv->stat == SERVER_RECONNECTING, G_SOURCE_REMOVE);

    srv->reconn_interval += SERVER_RECONN_STEP;
    srv->stat = SERVER_DISCONNECTED;
    server_connect(srv);

    return G_SOURCE_REMOVE;
}

void server_irc_event_connect(SircSession *sirc, const char *event){
    GSList *list;
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(srv->stat == SERVER_CONNECTING);

    /* Default state */
    srv->stat = SERVER_CONNECTED;
    srv->registered = FALSE;
    srv->loggedin = FALSE;
    srv->negotiated = FALSE;

    chat_add_misc_message_fmt(srv->chat, "", _("Connected to %1$s(%2$s:%3$d)"),
            srv->prefs->name, srv->prefs->host, srv->prefs->port);
    list = srv->chat_list;
    while (list){
        chat = list->data;
        chat_add_misc_message_fmt(chat, "", _("Connected to %1$s(%2$s:%3$d)"),
                srv->prefs->name, srv->prefs->host, srv->prefs->port);
        list = g_slist_next(list);
    }

    /* Start client capability negotiation */
    sirc_cmd_cap_ls(srv->irc, "302");

    if (!str_is_empty(srv->prefs->passwd)){
        /* Send connection password, you should send it command before sending
         * the NICK/USER combination. */
        sirc_cmd_pass(srv->irc, srv->prefs->passwd);
    }
    sirc_cmd_nick(srv->irc, srv->user->nick);
    sirc_cmd_user(srv->irc, srv->user->username, "hostname", "servername",
            srv->user->realname);
}

void server_irc_event_connect_fail(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    bool reconnect = FALSE;
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(srv->stat == SERVER_CONNECTING);

    const char *msg = count >= 1 ? params[0] : RET_MSG(SRN_ERR);

    switch (srv->disconn_reason){
        case SERVER_DISCONN_REASON_USER_CLOSE:
            // Nothing to do
            break;
        case SERVER_DISCONN_REASON_CLOSE:
            reconnect = TRUE;
            break;
        default:
            g_return_if_reached();
    }

    // Reset default disconnect reason
    srv->disconn_reason = SERVER_DISCONN_REASON_CLOSE;

    if (reconnect) {
        srv->stat = SERVER_RECONNECTING;
        // Add reconnect timer
        g_timeout_add(srv->reconn_interval, irc_reconnect, srv);
    } else {
        srv->stat = SERVER_DISCONNECTED;
    }

    chat_add_error_message_fmt(srv->chat, "", _("Failed to connect to %1$s(%2$s:%3$d): %4$s"),
            srv->prefs->name, srv->prefs->host, srv->prefs->port, msg);
    /* If user trying connect to a TLS port via non-TLS connection, it will
     * be reset, give user some hints. */
    if (!srv->prefs->irc->tls && srv->prefs->port == 6697) {
        chat_add_error_message_fmt(srv->chat, "",
                _("It seems that you connect to a TLS port(%1$d) without enable TLS connection, try to enable it and reconnect"),
                srv->prefs->port);
    }

    if (reconnect){
        chat_add_misc_message_fmt(srv->chat, "",
                _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                srv->prefs->name,
                srv->prefs->host,
                srv->prefs->port,
                (srv->reconn_interval * 1.0) / 1000);
    }
}

void server_irc_event_disconnect(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    bool reconnect = FALSE;
    GSList *list;
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(srv->stat == SERVER_CONNECTED);


    const char *msg = count >= 1 ? params[0] : RET_MSG(SRN_ERR);

    /* Update state */
    srv->registered = FALSE;
    srv->loggedin = FALSE;
    srv->negotiated = FALSE;

    /* Stop peroid ping */
    if (srv->ping_timer){
        DBG_FR("Ping timer %d removed", srv->ping_timer);

        g_source_remove(srv->ping_timer);
        srv->ping_timer = 0;
    }

    switch (srv->disconn_reason){
        case SERVER_DISCONN_REASON_USER_CLOSE:
            // Nothing to do
            break;
        case SERVER_DISCONN_REASON_QUIT:
            server_free(srv);
            return;
        case SERVER_DISCONN_REASON_TIMEOUT:
            msg = _("Ping time out");
            reconnect = TRUE;
            break;
        case SERVER_DISCONN_REASON_CLOSE:
            reconnect = TRUE;
            break;
        default:
            ERR_FR("Unknown disconnect reason: %d", srv->disconn_reason);
    }

    // Reset default disconnect reason
    srv->disconn_reason = SERVER_DISCONN_REASON_CLOSE;

    if (reconnect) {
        srv->stat = SERVER_RECONNECTING;
        // Add reconnect timer
        g_timeout_add(srv->reconn_interval, irc_reconnect, srv);
    } else {
        srv->stat = SERVER_DISCONNECTED;
    }

    /* Mark all channels as unjoined */
    list = srv->chat_list;
    while (list){
        Chat *chat;

        chat = list->data;
        if (sirc_is_chan(chat->name)){
            chat->joined = FALSE;
        }
        // Only report error message to server chat
        chat_add_misc_message_fmt(chat, "", _("Disconnected from %1$s(%2$s:%3$d): %4$s"),
                srv->prefs->name, srv->prefs->host, srv->prefs->port, msg);
        if (reconnect){
            chat_add_misc_message_fmt(chat, "",
                    _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                    srv->prefs->name,
                    srv->prefs->host,
                    srv->prefs->port,
                    (srv->reconn_interval * 1.0) / 1000);
        }

        list = g_slist_next(list);
    }

    chat_add_error_message_fmt(srv->chat, "", _("Disconnected from %1$s(%2$s:%3$d): %4$s"),
            srv->prefs->name, srv->prefs->host, srv->prefs->port, msg);
    if (reconnect){
        chat_add_misc_message_fmt(srv->chat, "",
                _("Trying reconnect to %1$s(%2$s:%3$d) after %4$.1lfs..."),
                srv->prefs->name,
                srv->prefs->host,
                srv->prefs->port,
                (srv->reconn_interval * 1.0) / 1000);
    }
}

void server_irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char **params, int count){
    GSList *list;
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));

    g_return_if_fail(count >= 1);
    const char *nick = params[0];

    /* You have registered when you recived a RPL_WELCOME(001) message */
    srv->registered = TRUE;

    /* Start peroid ping */
    srv->last_pong = get_time_since_first_call_ms();
    srv->ping_timer = g_timeout_add(SERVER_PING_INTERVAL, irc_period_ping, srv);
    DBG_FR("Ping timer %d created", srv->ping_timer);

    /* Resest reconnect interval */
    srv->reconn_interval = SERVER_RECONN_INTERVAL;

    user_rename(srv->user, nick);

    /* Join all channels already exists */
    list = srv->chat_list;
    while (list){
        Chat *chat = list->data;
        if (sirc_is_chan(chat->name)){
            sirc_cmd_join(srv->irc, chat->name, NULL);
        }
        list = g_slist_next(list);
    }
}

void server_irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    GSList *lst;
    Server *srv;
    Chat *chat;
    User *user;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *old_nick = origin;
    const char *new_nick = params[0];

    lst = srv->chat_list;
    while (lst){
        chat = lst->data;
        // TODO: dialog nick track support
        if ((user = chat_get_user(chat, old_nick)) != NULL){
            chat_add_misc_message_fmt(chat, old_nick, _("%1$s is now known as %2$s"),
                    old_nick, new_nick);
            user_rename(user, new_nick);
        }
        lst = g_slist_next(lst);
    }

    if (sirc_nick_cmp(old_nick, srv->user->nick)){
        chat_add_misc_message_fmt(srv->chat, old_nick, _("%1$s is now known as %2$s"),
                old_nick, new_nick);
        user_rename(srv->user, new_nick);
    }
}

void server_irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    GSList *lst;
    Server *srv;
    Chat *chat;
    User *user;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *reason = params[0];

    if (reason) {
        snprintf(buf, sizeof(buf), _("%1$s has quit: %2$s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%1$s has quit"), origin);
    }

    lst = srv->chat_list;
    while (lst){
        // TODO: dialog support
        chat = lst->data;
        if ((user = chat_get_user(chat, origin)) != NULL){
            chat_add_misc_message(chat, origin, buf);
            chat_rm_user(chat, origin);
        }
        lst = g_slist_next(lst);
    }

    /* You quit */
    if (sirc_nick_cmp(origin, srv->user->nick)){
        server_disconnect(srv, SERVER_DISCONN_REASON_QUIT);
    }
}

void server_irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    bool youjoin;
    char buf[512];
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *chan = params[0];

    /* You has join a channel */
    youjoin = sirc_nick_cmp(srv->user->nick, origin);

    if (youjoin) {
        server_add_chat(srv, chan);
    }

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    if (youjoin) {
        chat->joined = TRUE;
    }

    if (youjoin) {
        snprintf(buf, sizeof(buf), _("You has joined"));
    } else {
        snprintf(buf, sizeof(buf), _("%1$s has joined"), origin);
        chat_add_user(chat, origin, USER_CHIGUA);
    }

    chat_add_misc_message(chat, origin, buf);
}

void server_irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *chan = params[0];
    const char *reason = count == 2 ? params[1] : NULL;

    if (reason){
        snprintf(buf, sizeof(buf), _("%1$s has left: %2$s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%1$s has left"), origin);
    }

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    chat_add_misc_message(chat, origin, buf);
    chat_rm_user(chat, origin);

    /* You has left a channel */
    if (strncasecmp(srv->user->nick, origin, NICK_LEN) == 0){
        chat->joined = FALSE;
        server_rm_chat(srv, chan);
    }
}

void server_irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    GString *buf;
    GString *modes;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 1);

    const char *chan = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    modes = g_string_new(NULL);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(modes, "%s ", params[i]);
    }

    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %1$s %2$s by %3$s"), chan, modes->str, origin);

    chat_add_misc_message(chat, origin, buf->str);

    // FIXME: Only process the first mode
    do {
        if (count < 3) break;
        const char *mode = params[1];
        const char *mode_args = params[2];

        User *user = chat_get_user(chat, mode_args);
        if (!user) break;

        if (strlen(mode) < 2) break;

        if (mode[0] == '-'){
            user_set_type(user, USER_CHIGUA);
        }
        else if (mode[0] == '+'){
            switch (mode[1]){
                case 'q':
                    user_set_type(user, USER_OWNER);
                    break;
                case 'a':
                    user_set_type(user, USER_ADMIN);
                    break;
                case 'o':
                    user_set_type(user, USER_FULL_OP);
                    break;
                case 'h':
                    user_set_type(user, USER_HALF_OP);
                    break;
                case 'v':
                    user_set_type(user, USER_VOICED);
                    break;
                default:
                    break;
            }
        } else {
            ERR_FR("Unrecognized mode: %s. chan: %s, mode_args: %s",
                    mode, chan, mode_args);
        }
    } while (0);

    g_string_free(modes, TRUE);
    g_string_free(buf, TRUE);
}

void server_irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    GString *buf;
    GString *modes;
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *nick = params[0];

    modes = g_string_new(NULL);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(modes, "%s ", params[i]);
    }

    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %1$s %2$s by %3$s"), nick, modes->str, origin);

    chat_add_misc_message(srv->chat, origin, buf->str);

    g_string_free(modes, TRUE);
    g_string_free(buf, TRUE);
}

void server_irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 2);

    const char *chan = params[0];
    const char *topic = params[1];

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    chat_set_topic(chat, origin, topic);

    if (strlen(topic) == 0) {
        chat_add_misc_message_fmt(chat, origin, _("%1$s cleared topic"),
                origin, topic);
    } else {
        chat_add_misc_message_fmt(chat, origin, _("%1$s changed topic to:\n\t%2$s"),
                origin, topic);
    }
}

void server_irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    char buf[512];
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 2);

    const char *chan = params[0];
    const char *kick_nick = params[1];
    const char *reason = count == 3 ? params[2] : NULL;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    if (strncasecmp(kick_nick, srv->user->nick, NICK_LEN) == 0){
        /* You are kicked 23333 */
        if (reason){
            snprintf(buf, sizeof(buf), _("You are kicked by %1$s: %2$s"),
                    kick_nick, reason);
        } else {
            snprintf(buf, sizeof(buf), _("You are kicked by %1$s"),
                    kick_nick);
        }
        chat->joined = FALSE;
    } else {
        if (reason){
            snprintf(buf, sizeof(buf), _("%1$s are kicked by %2$s: %3$s"),
                    kick_nick, origin, reason);
        } else {
            snprintf(buf, sizeof(buf), _("%1$s are kicked by %2$s"),
                    kick_nick, origin);
        }
    }

    chat_add_misc_message(chat, kick_nick, buf);
    chat_rm_user(chat, kick_nick);
}

void server_irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;
    User *user;

    g_return_if_fail(count >= 2);

    const char *chan = params[0];
    const char *msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);
    user = chat_get_user(chat, origin);
    g_return_if_fail(user);

    chat_add_recv_message(chat, origin, msg);
}

void server_irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 2);
    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    chat = server_get_chat_fallback(srv, origin);
    g_return_if_fail(chat);

    const char *msg = params[1];

    chat_add_recv_message(chat, origin, msg);
}

void server_irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 2);
    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    chat = server_get_chat_fallback(srv, origin);
    g_return_if_fail(chat);

    const char *msg = params[1];

    chat_add_notice_message(chat, origin, msg);
}

void server_irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 2);
    const char *chan = params[0];
    const char *msg = params[1];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    chat = server_get_chat_fallback(srv, chan);
    g_return_if_fail(chat);

    chat_add_notice_message(chat, origin, msg);
}

void server_irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 2);

    const char *chan = params[1];

    chat_add_misc_message_fmt(srv->chat, origin, _("%1$s invites you into %2$s"),
            origin, chan);
    // TODO: Info bar
}

void server_irc_event_ctcp_req(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *target = params[0];
    const char *msg = count == 2 ? params[1] : NULL;

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

    if (sirc_is_chan(target)){
        chat = server_get_chat(srv, target);
    } else {
        chat = server_get_chat_fallback(srv, origin);
    }
    g_return_if_fail(chat);

    /* Show it on ui */
    if (strcmp(event, "ACTION") == 0) {
        chat_add_action_message(chat, origin, msg);
    } else if (strcmp(event, "DCC") == 0) {
        chat_add_error_message_fmt(chat, origin,
                _("Received unsupported CTCP %1$s request form %2$s"),
                event, origin);
    } else {
        chat_add_misc_message_fmt(chat, origin,
                _("Received CTCP %1$s request form %2$s"), event, origin);
    }
}

void server_irc_event_ctcp_rsp(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 2);

    const char *target = params[0];
    const char *msg = params[1];

    if (sirc_is_chan(target)){
        chat = server_get_chat(srv, target);
    } else {
        chat = server_get_chat_fallback(srv, origin);
    }
    g_return_if_fail(chat);

    if (strcmp(event, "ACTION") == 0) {
        // Is there any ACTION response?
    } else if (strcmp(event, "CLIENTINFO") == 0
            || strcmp(event, "FINGER") == 0
            || strcmp(event, "SOURCE") == 0
            || strcmp(event, "TIME") == 0
            || strcmp(event, "VERSION") == 0
            || strcmp(event, "USERINFO") == 0){
        chat_add_misc_message_fmt(chat, origin,
                _("Received CTCP %1$s response from %2$s: %3$s"), event, origin, msg);
    } else if (strcmp(event, "DCC") == 0) {
        // TODO
        chat_add_error_message_fmt(chat, origin,
                _("Received unsupported CTCP %1$s response form %2$s"),
                event, origin);
    } else if (strcmp(event, "PING") == 0) {
        unsigned long time;
        unsigned long nowtime;

        time = strtoul(msg, NULL, 10);
        nowtime = get_time_since_first_call_ms();

        if (time != 0 && nowtime >= time){
            /* Update dalay and pong time */
            chat_add_misc_message_fmt(chat, origin,
                    _("Latency between %1$s: %2$lums"), origin, nowtime - time);
        } else {
        chat_add_misc_message_fmt(chat, origin,
                _("Received CTCP %1$s response from %2$s: %3$s"), event, origin, msg);
        }
    } else {
        WARN_FR("Unknown CTCP message: %s", event);
    }
}

void server_irc_event_cap(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count){
    bool multiline;
    bool cap_end;
    const char *cap_event;
    const char *rawcaps;
    char **caps;
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 3);

    cap_end = FALSE;
    cap_event = params[1];
    rawcaps = params[2];

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
            if (server_cap_is_support(srv->cap, name, value)
                    && RET_IS_OK(server_cap_server_enable(srv->cap, name, TRUE))){
                g_string_append_printf(buf, "%s ", name);
            }
        }

        chat_add_misc_message_fmt(srv->chat, origin,
                _("Server capabilities: %1$s"), rawcaps);
        if (buf->len > 0){
            chat_add_misc_message_fmt(srv->chat, origin,
                    _("Requesting capabilities: %1$s"), buf->str);
            sirc_cmd_cap_req(sirc, buf->str);
        } else {
            chat_add_misc_message_fmt(srv->chat, origin,
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
            if (server_cap_is_support(srv->cap, name, value)
                    && RET_IS_OK(server_cap_server_enable(srv->cap, name, TRUE))){
                g_string_append_printf(buf, "%s ", name);
            }
        }

        chat_add_misc_message_fmt(srv->chat, origin,
                _("Server has new capabilities: %1$s"), rawcaps);
        if (buf->len > 0){
            chat_add_misc_message_fmt(srv->chat, origin,
                    _("Requesting new capabilities: %1$s"), buf->str);
            sirc_cmd_cap_req(sirc, buf->str);
        } else {
            chat_add_misc_message_fmt(srv->chat, origin,
                    _("No new capability to be requested"));
        }

        g_string_free(buf, TRUE);
    } else if (g_ascii_strcasecmp(cap_event, "LIST") == 0){
        if (strlen(rawcaps) > 0){
            chat_add_misc_message_fmt(srv->chat, origin,
                    _("Acknowledged capabilities: %1$s"), rawcaps);
        } else {
            chat_add_misc_message_fmt(srv->chat, origin,
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

            if (!RET_IS_OK(server_cap_client_enable(srv->cap, name, enable))){
                WARN_FR("Unknown capability: %s", name);
            }

            if (server_cap_all_enabled(srv->cap)){
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

            if (!RET_IS_OK(server_cap_client_enable(srv->cap, name, enable))){
                WARN_FR("Unknown capability: %s", name);
            }
        }

        chat_add_misc_message_fmt(srv->chat, origin,
                _("Server has deleted capabilities: %1$s"), rawcaps);
    } else if (g_ascii_strcasecmp(cap_event, "NAK") == 0){
        cap_end = TRUE;
    } else {
        g_warn_if_reached();
    }

    /* Whether to end the negotiation? */
    if (!srv->negotiated && cap_end){
        sirc_cmd_cap_list(sirc);

        if (srv->prefs->login_method == LOGIN_SASL_PLAIN){
            if (srv->cap->client_enabled.sasl){
                // Negotiation should end after sasl authentication end
            } else {
                chat_add_error_message_fmt(srv->chat, origin,
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

void server_irc_event_authenticate(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));

    switch (srv->prefs->login_method){
        case LOGIN_SASL_PLAIN:
            {
                char *base64;
                char *login_method;
                GString *str;

                g_return_if_fail(!str_is_empty(srv->prefs->username));
                g_return_if_fail(!str_is_empty(srv->prefs->user_passwd));

                /* ref: https://ircv3.net/specs/extensions/sasl-3.1.html */
                str = g_string_new(NULL);
                str = g_string_append(str, srv->prefs->username);
                str = g_string_append_unichar(str, g_utf8_get_char("\0")); // Unicode null char
                str = g_string_append(str, srv->prefs->username);
                str = g_string_append_unichar(str, g_utf8_get_char("\0")); // Unicode null char
                str = g_string_append(str, srv->prefs->user_passwd);

                // TODO: 400 bytes limit
                base64 = g_base64_encode((const guchar *)str->str, str->len);
                sirc_cmd_authenticate(sirc, base64);

                login_method = login_method_to_string(srv->prefs->login_method);
                chat_add_misc_message_fmt(srv->chat, origin,
                        _("Logging in with %1$s as %2$s..."),
                        login_method, srv->prefs->username);

                g_free(login_method);
                g_free(base64);
                g_string_free(str, TRUE);
                break;
            }
        default:
            g_warn_if_reached();
    }
}

void server_irc_event_ping(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    /* Nothing to do
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    */
}

void server_irc_event_pong(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;
    unsigned long time;
    unsigned long nowtime;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);


    const char *msg = params[count - 1]; // time should be the last parameter

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

void server_irc_event_error(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));
    g_return_if_fail(count >= 1);

    const char *msg = params[0];

    chat_add_error_message_fmt(srv->cur_chat, origin, _("ERROR: %1$s"), msg);
}

void server_irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(server_is_valid(srv));

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

                chat_add_recv_message(srv->chat, origin, buf->str);

                g_string_free(buf, TRUE);

                break;
            }
        case SIRC_RFC_ERR_NICKNAMEINUSE:
            {
                g_return_if_fail(count >= 3);

                const char *nick = params[1];

                /* If you don't have a nickname (unregistered) yet, try a nick
                 * with a trailing underline('_') */
                if (!srv->registered){
                    char *new_nick = g_strdup_printf("%s_", nick);

                    // FIXME: ircd-seven will truncate the nick without
                    // returning a error message if it reaches the length
                    // limiation, at this time the new_nick is same to the
                    // registered old nick in the server view.
                    sirc_cmd_nick(srv->irc, new_nick);
                    chat_add_misc_message_fmt(srv->cur_chat, origin,
                            _("Trying nickname: \"%1$s\"..."), new_nick);

                    g_free(new_nick);
                }
                goto ERRMSG;
            }

            /************************ NAMES message ************************/
        case SIRC_RFC_RPL_NAMREPLY:
            {
                g_return_if_fail(count >= 4);

                char *nickptr;
                const char *chan = params[2];
                char *names = (char *)params[3];
                Chat *chat;
                UserType type;

                chat = server_get_chat(srv, chan);
                g_return_if_fail(chat);

                nickptr = strtok(names, " ");
                while (nickptr){
                    switch (nickptr[0]){
                        case '~':
                            nickptr++;
                            type = USER_OWNER;
                            break;
                        case '&':
                            nickptr++;
                            type = USER_ADMIN;
                            break;
                        case '@':
                            nickptr++;
                            type = USER_FULL_OP;
                            break;
                        case '%':
                            nickptr++;
                            type = USER_HALF_OP;
                            break;
                        case '+':
                            nickptr++;
                            type = USER_VOICED;
                            break;
                        default:
                            type = USER_CHIGUA;
                    }
                    chat_add_user(chat, nickptr, type);
                    nickptr = strtok(NULL, " ");
                }
                break;
            }
        case SIRC_RFC_RPL_ENDOFNAMES:
            break;

        case SIRC_RFC_RPL_NOTOPIC:
            {
                g_return_if_fail(count >= 2);
                const char *chan = params[1];

                Chat *chat = server_get_chat(srv, chan);
                g_return_if_fail(chat);

                chat_add_misc_message_fmt(chat, origin,
                        _("No topic is set"), chan);

                break;
            }
        case SIRC_RFC_RPL_TOPIC:
            {
                g_return_if_fail(count >= 3);

                const char *chan = params[1];
                const char *topic =params[2];

                Chat *chat = server_get_chat(srv, chan);
                g_return_if_fail(chat);

                chat_set_topic(chat, origin, topic);
                chat_add_misc_message_fmt(chat, origin,
                        _("The topic of this channel is:\n\t%1$s"), topic);

                break;
            }
        case SIRC_RFC_RPL_TOPICWHOTIME:
            {
                g_return_if_fail(count >= 4);
                char timestr[64];
                const char *chan = params[1];
                const char *who = params[2];
                time_t time = strtoul(params[3], NULL, 10);

                Chat *chat = server_get_chat(srv, chan);
                g_return_if_fail(chat);

                time_to_str(time, timestr, sizeof(timestr), _("%Y-%m-%d %T"));
                char *setter = g_strdup_printf(_("By %1$s at %2$s"), who, timestr);
                chat_set_topic_setter(chat, setter);
                g_free(setter);

                break;
            }

            /************************ WHOIS message ************************/
        case SIRC_RFC_RPL_WHOISUSER:
            {
                g_return_if_fail(count >= 6);

                const char *nick = params[1];
                const char *user = params[2];
                const char *host = params[3];
                const char *realname = params[4];
                chat_add_misc_message_fmt(srv->cur_chat, origin, "%s <%s@%s> %s",
                        nick, user, host, realname);
                // TODO pass NULL ui to send to current chat
                plugin_avatar(nick, realname);
                break;
            }
        case SIRC_RFC_RPL_WHOISCHANNELS:
            {
                g_return_if_fail(count >= 3);

                const char *msg = params[2];

                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%1$s is member of %2$s"), params[1], msg);
                break;
            }
        case SIRC_RFC_RPL_WHOISSERVER:
            {
                g_return_if_fail(count >= 4);

                const char *msg = params[3];

                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%1$s is attached to %2$s at \"%3$s\""),
                        params[1], params[2], msg);
                break;
            }
        case SIRC_RFC_RPL_WHOISIDLE:
            {
                g_return_if_fail(count >= 4);
                const char *who = params[1];
                const char *sec = params[2];
                time_t since = strtoul(params[3], NULL, 10);
                char timestr[64];
                time_to_str(since, timestr, sizeof(timestr), _("%Y-%m-%d %T"));
                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%1$s is idle for %2$s seconds since %3$s"),
                        who, sec, timestr);
                break;
            }
        case SIRC_RFC_RPL_WHOWAS_TIME:
            {
                g_return_if_fail(count >= 4);

                const char *msg = params[3];

                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%1$s %2$s %3$s"),
                        params[1], msg, params[2]);
                break;
            }
        case SIRC_RFC_RPL_WHOISHOST:
        case SIRC_RFC_RPL_WHOISSECURE:
            {
                g_return_if_fail(count >= 3);

                const char *msg = params[2];

                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%1$s %2$s"), params[1], msg);
                break;
            }
        case SIRC_RFC_RPL_ENDOFWHOIS:
            {
                g_return_if_fail(count >= 3);

                const char *msg = params[2];

                chat_add_misc_message(srv->cur_chat, origin, msg);
                break;
            }

            /************************ WHO message ************************/
        case SIRC_RFC_RPL_WHOREPLY:
            {
                g_return_if_fail(count >= 7);

                /* params[count - 1] = "<hopcount> <realname>", Skip ' ' */
                const char *nick = params[5];
                const char *realname = strchr(params[6], ' ');

                /* Use Real Name as avatar token :-( */
                plugin_avatar(nick, realname);
                break;
            }
        case SIRC_RFC_RPL_ENDOFWHO:
            break;
            /************************ BANLIST message ************************/
        case SIRC_RFC_RPL_BANLIST:
            {
                g_return_if_fail(count >= 3);

                const char *chan = params[1];
                const char *banmask = params[2];

                Chat *chat = server_get_chat(srv, chan);
                if (!chat) chat = srv->cur_chat;

                chat_add_misc_message_fmt(srv->cur_chat, origin, "%s: %s",
                        chan, banmask);
                // TODO: <time_left> and <reason> are not defined in RFC
                break;
            }
        case SIRC_RFC_RPL_ENDOFBANLIST:
            {
                g_return_if_fail(count >= 3);

                const char *chan = params[1];
                const char *msg = params[2];

                Chat *chat = server_get_chat(srv, chan);
                if (!chat) chat = srv->cur_chat;

                chat_add_misc_message_fmt(chat, origin, _("%1$s: %2$s"), chan, msg);
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
                g_return_if_fail(count >= 4);

                const char *chan = params[1];
                int users = atoi(params[2]);
                const char *topic = params[3];

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
                g_return_if_fail(count >= 4);

                const char *msg = params[3];

                srv->loggedin = TRUE;
                sirc_cmd_cap_end(sirc); // End negotiation
                chat_add_recv_message(srv->chat, origin, msg);
                break;
            }
        case SIRC_RFC_RPL_SASLSUCCESS:
            {
                g_return_if_fail(count >= 2);

                const char *msg = params[1];

                chat_add_recv_message(srv->chat, origin, msg);
                break;
            }
        case SIRC_RFC_RPL_LOGGEDOUT:
            {
                g_return_if_fail(count >= 3);

                const char *msg = params[2];

                srv->loggedin = FALSE;
                chat_add_recv_message(srv->chat, origin, msg);
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
                g_return_if_fail(count >= 3);

                const char *chan = params[1];
                const char *msg = params[2];

                Chat *chat = server_get_chat(srv, chan);
                if (!chat) chat = srv->cur_chat;

                chat_add_misc_message_fmt(chat, origin, _("URL of %1$s is: %2$s"),
                        chan, msg);
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
                            srv->prefs->name, event, origin, count, buf->str);

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

                    chat_add_error_message_fmt(srv->cur_chat, origin,
                            _("ERROR[%1$3d] %2$s"), event, buf->str);

                    g_string_free(buf, TRUE);
                }
                return;
            }
    }
}
