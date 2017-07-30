/**
 * @file server_irc_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 *
 */


#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <strings.h>

#include "server.h"
#include "server_irc_event.h"

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
    g_return_val_if_fail(srv, G_SOURCE_REMOVE);
    // g_return_if_fail(server_is_vaild(srv));

    time = get_time_since_first_call_ms();
    snprintf(timestr, sizeof(timestr), "%lu", time);

    DBG_FR("%lu ms since last pong, time out: %d ms",
            time - srv->last_pong, SERVER_PING_TIMEOUT);

    /* Check whether ping time out */
    if (time - srv->last_pong > SERVER_PING_TIMEOUT){
        /* Reconnect */
        WARN_FR("Reconnecting to %s...", srv->prefs->name);
        server_disconnect(srv);
        server_connect(srv);

        return G_SOURCE_REMOVE;
    }

    sirc_cmd_ping(srv->irc, timestr);

    return G_SOURCE_CONTINUE;
}

void server_irc_event_connect(SircSession *sirc, const char *event){
    GSList *list;
    Server *srv;
    User *user;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);

    user = srv->user;
    srv->stat = SERVER_CONNECTED;
    chat_add_misc_message_fmt(srv->chat, "", _("Connected to %s(%s:%d)"),
            srv->prefs->name, srv->prefs->host, srv->prefs->port);

    if (srv->prefs->passwd){
        /* Send connection password, you should send it command before sending
         * the NICK/USER combination. */
        sirc_cmd_pass(srv->irc, srv->prefs->passwd);
    }
    sirc_cmd_nick(srv->irc, user->nick);
    sirc_cmd_user(srv->irc, user->username, "hostname", "servername", user->realname);

    /* Start peroid ping */
    srv->last_pong = get_time_since_first_call_ms();
    srv->ping_timer = g_timeout_add(SERVER_PING_INTERVAL, irc_period_ping, srv);

    /* Join all channels already exists */
    list = srv->chat_list;
    while (list){
        chat = list->data;

        if (sirc_is_chan(chat->name)){
            sirc_cmd_join(srv->irc, chat->name, NULL);
        }
        chat_add_misc_message_fmt(chat, "", _("Connected to %s(%s:%d)"),
                srv->prefs->name, srv->prefs->host, srv->prefs->port);

        list = g_slist_next(list);
    }

    DBG_FR("Ping timer %d created", srv->ping_timer);
}

void server_irc_event_disconnect(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    GSList *list;
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);

    srv->stat = SERVER_DISCONNECTED;

    /* Stop peroid ping */
    if (srv->ping_timer){
        g_source_remove(srv->ping_timer);

        DBG_FR("Ping timer %d removed", srv->ping_timer);

        srv->ping_timer = 0;
    }

    /* Unjoin all channels */
    list = srv->chat_list;
    while (list){
        chat = list->data;
        if (sirc_is_chan(chat->name)){
            chat->joined = FALSE;
        }
        if (msg){
            /* Only report error message to server chat */
            chat_add_misc_message_fmt(chat, "", _("Disconnected from %s(%s:%d): %s"),
                    srv->prefs->name, srv->prefs->host, srv->prefs->port, msg);
        } else {
            chat_add_misc_message_fmt(chat, "", _("Disconnected from %s(%s:%d)"),
                    srv->prefs->name, srv->prefs->host, srv->prefs->port);
        }

        list = g_slist_next(list);
    }

    if (msg){
        /* Disconnected because of some error */
        chat_add_error_message_fmt(srv->chat, "", _("Disconnected from %s(%s:%d): %s"),
                srv->prefs->name, srv->prefs->host, srv->prefs->port, msg);

        /* If user trying connect to a TLS port via non-TLS connection, it will
         * be reset, give user some hints. */
        if (!srv->prefs->irc->tls && srv->prefs->port == 6697) {
            chat_add_error_message_fmt(srv->chat, "",
                    _("It seems that you connect to a TLS port(%d) without enable TLS connection, try to enable it and reconnect"),
                    srv->prefs->port);
        }
    } else {
        /* Peacefully disconnected */
        chat_add_misc_message_fmt(srv->chat, "", _("Disconnected from %s(%s:%d)"),
                srv->prefs->name, srv->prefs->host, srv->prefs->port);
    }

    if (srv->user_quit){
        server_free(srv);
    } else {
        // TODO: reconnect logic
        // server_connect(srv);
    }
}

void server_irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);

    g_return_if_fail(count >= 1);

    const char *nick = params[0];

    /* You have registered when you recived a RPL_WELCOME(001) message */
    srv->registered = TRUE;
    srv->user_quit = FALSE;

    user_rename(srv->user, nick);
}

void server_irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    GSList *lst;
    Server *srv;
    Chat *chat;
    User *user;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(msg);

    const char *old_nick = origin;
    const char *new_nick = msg;

    lst = srv->chat_list;
    while (lst){
        chat = lst->data;
        // TODO: dialog nick track support
        if ((user = chat_get_user(chat, old_nick)) != NULL){
            chat_add_misc_message_fmt(chat, old_nick, _("%s is now known as %s"),
                    old_nick, new_nick);
            user_rename(user, new_nick);
        }
        lst = g_slist_next(lst);
    }

    if (sirc_nick_cmp(old_nick, srv->user->nick)){
        chat_add_misc_message_fmt(srv->chat, old_nick, _("%s is now known as %s"),
                old_nick, new_nick);
        user_rename(srv->user, new_nick);
    }
}

void server_irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    GSList *lst;
    Server *srv;
    Chat *chat;
    User *user;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(msg);

    const char *reason = msg;

    if (reason) {
        snprintf(buf, sizeof(buf), _("%s has quit: %s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%s has quit"), origin);
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
        srv->registered = FALSE;
        srv->user_quit = TRUE;

        server_disconnect(srv);
    }
}

void server_irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    bool youjoin;
    char buf[512];
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(msg || count >= 1);

    const char *chan = msg ? msg : params[0];

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
        snprintf(buf, sizeof(buf), _("%s has joined"), origin);
        chat_add_user(chat, origin, USER_CHIGUA);
    }

    chat_add_misc_message(chat, origin, buf);
}

void server_irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(count >= 1);

    const char *chan = params[0];
    const char *reason = msg;

    if (reason){
        snprintf(buf, sizeof(buf), _("%s has left: %s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%s has left"), origin);
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
        const char *origin, const char **params, int count, const char *msg){
    GString *buf;
    GString *modes;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 1);

    const char *chan = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    modes = g_string_new(NULL);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(modes, "%s ", params[i]);
    }
    if (msg) {
        g_string_append_printf(modes, "%s ", msg);
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
        const char *origin, const char **params, int count, const char *msg){
    GString *buf;
    GString *modes;
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(count >= 1);

    const char *nick = params[0];

    modes = g_string_new(NULL);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(modes, "%s ", params[i]);
    }
    if (msg) {
        g_string_append_printf(modes, "%s ", msg);
    }

    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %1$s %2$s by %3$s"), nick, modes->str, origin);

    chat_add_misc_message(srv->chat, origin, buf->str);

    g_string_free(modes, TRUE);
    g_string_free(buf, TRUE);
}

void server_irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(count >= 1);
    g_return_if_fail(msg);

    const char *chan = params[0];
    const char *topic = msg;

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    chat_set_topic(chat, origin, topic);
    chat_add_misc_message_fmt(chat, origin, _("%s changed topic to:\n\t%s"),
            origin, topic);
}

void server_irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 2);

    const char *chan = params[0];
    const char *kick_nick = count >= 2 ? params[1] : ""; // TODO: ???
    const char *reason = msg;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    if (strncasecmp(kick_nick, srv->user->nick, NICK_LEN) == 0){
        /* You are kicked 23333 */
        if (reason){
            snprintf(buf, sizeof(buf), _("You are kicked by %s: %s"),
                    kick_nick, reason);
        } else {
            snprintf(buf, sizeof(buf), _("You are kicked by %s"),
                    kick_nick);
        }
        chat->joined = FALSE;
    } else {
        if (reason){
            snprintf(buf, sizeof(buf), _("%s are kicked by %s: %s"),
                    kick_nick, origin, reason);
        } else {
            snprintf(buf, sizeof(buf), _("%s are kicked by %s"),
                    kick_nick, origin);
        }
    }

    chat_add_misc_message(chat, kick_nick, buf);
    chat_rm_user(chat, kick_nick);
}

void server_irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;
    User *user;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);

    const char *chan = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);
    user = chat_get_user(chat, origin);
    g_return_if_fail(user);

    chat_add_recv_message(chat, origin, msg);
}

void server_irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 0);
    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    chat = server_get_chat_fallback(srv, origin);
    g_return_if_fail(chat);

    chat_add_recv_message(chat, origin, msg);
}

void server_irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);

    chat = server_get_chat_fallback(srv, origin);
    g_return_if_fail(chat);

    chat_add_notice_message(chat, origin, msg);
}

void server_irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);
    const char *chan = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    chat = server_get_chat_fallback(srv, chan);
    g_return_if_fail(chat);

    chat_add_notice_message(chat, origin, msg);
}

void server_irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(count >= 2);

    const char *chan = params[1];

    chat_add_misc_message_fmt(srv->chat, origin, _("%s invites you into %s"),
            origin, chan);
    // TODO: Info bar
}

void server_irc_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);
    const char *target = params[0];

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);

    if (sirc_is_chan(target)){
        chat = server_get_chat(srv, target);
    } else {
        chat = server_get_chat_fallback(srv, origin);
    }

    g_return_if_fail(chat);

    chat_add_action_message(chat, origin, msg);
}

void server_irc_event_ping(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    /* Nothing to do
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    */
}

void server_irc_event_pong(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    unsigned long time;
    unsigned long nowtime;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(msg);

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
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);
    g_return_if_fail(msg);

    chat_add_error_message_fmt(srv->cur_chat, origin, _("ERROR: %s"), msg);
}

void server_irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;

    srv = sirc_get_ctx(sirc);
    g_return_if_fail(srv);

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
                if (msg) g_string_append_printf(buf, "%s", msg);

                chat_add_recv_message(srv->chat, origin, buf->str);

                g_string_free(buf, TRUE);

                break;
            }
        case SIRC_RFC_ERR_NICKNAMEINUSE:
            {
                g_return_if_fail(msg);

                chat_add_error_message_fmt(srv->cur_chat, origin,
                        _("ERROR[%3d]: %s"), event, msg);

                /* If you don't have a nickname (unregistered) yet, try a nick
                 * with a trailing underline('_') */
                if (!srv->registered){
                    int len = strlen(srv->user->nick);

                    if (len < sizeof(srv->user->nick) - 1){
                        char new_nick[NICK_LEN];

                        g_strlcpy(new_nick, srv->user->nick, sizeof(new_nick));

                        new_nick[len] = '_';
                        new_nick[len + 1] = '\0';
                        sirc_cmd_nick(srv->irc, new_nick);

                        chat_add_misc_message_fmt(srv->cur_chat, origin,
                                _("Trying nickname: \"%s\"..."), new_nick);
                    } else {
                        chat_add_error_message_fmt(srv->cur_chat, origin,
                                _("Your nickname is too long to add a trailing underline"));
                    }
                }
                break;
            }

            /************************ NAMES message ************************/
        case SIRC_RFC_RPL_NAMREPLY:
            {
                g_return_if_fail(count >= 3);
                g_return_if_fail(msg);
                char *nickptr;
                const char *chan = params[2];
                char *names = (char *)msg; // TODO: ...
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

        case SIRC_RFC_RPL_TOPIC:
            {
                g_return_if_fail(count >= 2);
                g_return_if_fail(msg);
                const char *chan = params[1];
                const char *topic = msg;

                Chat *chat = server_get_chat(srv, chan);
                g_return_if_fail(chat);

                chat_set_topic(chat, origin, topic);

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
                char *setter = g_strdup_printf(_("By %s at %s"), who, timestr);
                chat_set_topic_setter(chat, setter);
                g_free(setter);
            }

            /************************ WHOIS message ************************/
        case SIRC_RFC_RPL_WHOISUSER:
            {
                g_return_if_fail(count >= 5);
                g_return_if_fail(msg);
                const char *nick = params[1];
                const char *user = params[2];
                const char *host = params[3];
                const char *realname = msg;
                chat_add_misc_message_fmt(srv->cur_chat, origin, "%s <%s@%s> %s",
                        nick, user, host, realname);
                // TODO pass NULL ui to send to current chat
                plugin_avatar(nick, realname);
                break;
            }
        case SIRC_RFC_RPL_WHOISCHANNELS:
            {
                g_return_if_fail(count >= 2);
                g_return_if_fail(msg);
                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%s is member of %s"), params[1], msg);
                break;
            }
        case SIRC_RFC_RPL_WHOISSERVER:
            {
                g_return_if_fail(count >= 3);
                g_return_if_fail(msg);
                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%s is attached to %s at \"%s\""),
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
                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%s is idle for %s seconds since %s"),
                        who, sec, since);
                break;
            }
        case SIRC_RFC_RPL_WHOWAS_TIME:
            {
                g_return_if_fail(count >= 3);
                g_return_if_fail(msg);
                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%s %s %s"),
                        params[1], msg, params[2]);
                break;
            }
        case SIRC_RFC_RPL_WHOISHOST:
        case SIRC_RFC_RPL_WHOISSECURE:
            {
                g_return_if_fail(count >= 2);
                g_return_if_fail(msg);
                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%s %s"), params[1], msg);
                break;
            }
        case SIRC_RFC_RPL_ENDOFWHOIS:
            {
                g_return_if_fail(msg);
                chat_add_misc_message(srv->cur_chat, origin, msg);
                break;
            }

            /************************ WHO message ************************/
        case SIRC_RFC_RPL_WHOREPLY:
            {
                g_return_if_fail(count >= 6);
                g_return_if_fail(msg);
                /* params[count - 1] = "<hopcount> <realname>", Skip ' ' */
                const char *nick = params[5];
                const char *realname = strchr(msg, ' ');
                if (realname) realname++;
                else break;
                /* Use Real Name as avatar token :-( */
                plugin_avatar(nick, realname);
                break;
            }
        case SIRC_RFC_RPL_ENDOFWHO:
            break;
            /************************ BANLIST message ************************/
        case SIRC_RFC_RPL_BANLIST:
            {
                srv = sirc_get_ctx(sirc);
                g_return_if_fail(srv);
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
                srv = sirc_get_ctx(sirc);
                g_return_if_fail(srv);
                g_return_if_fail(count >= 2);
                g_return_if_fail(msg);

                const char *chan = params[1];

                Chat *chat = server_get_chat(srv, chan);
                if (!chat) chat = srv->cur_chat;

                chat_add_misc_message_fmt(srv->cur_chat, origin, _("%s: %s"),
                        chan,msg);
                break;
            }
        default:
            {
                // Error message
                if (event >= 400 && event < 600){
                    g_return_if_fail(msg);
                    chat_add_error_message_fmt(srv->cur_chat, origin, _("ERROR[%3d]: %s"), event, msg);
                    return;
                }

                WARN_FR("Unspported message, You can report it at " PACKAGE_WEBSITE);
                WARN_FR("server: %s, event: %d, origin: %s, count: %u, params: [",
                        srv->prefs->name, event, origin, count);
                for (int i = 0; i < count; i++) LOG("'%s', ", params[i]); LOG("]\n");
            }
    }
}
