/**
 * @file irc_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 *
 * TODO: 不要相信任何来自 IRC Server 的数据，各种 check 待补
 */

#define __DBG_ON
#define __LOG_ON

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <strings.h>

#include "srv.h"
#include "irc_event.h"

#include "sui/sui.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "filter.h"
#include "meta.h"
#include "chat_log.h"
#include "plugin.h"

#define PRINT_EVENT_PARAM \
    do { \
        DBG_FR("server: %s, event: %s, origin: %s", srv->name, event, origin); \
        DBG_FR("msg: %s", msg); \
        for (int i = 0; i < count; i++){ \
            if (i == 0) DBG_F("count: %d, params: [", count); \
            if (i == count - 1) { \
                DBG("%s]\n", params[i]); \
            } else { \
                DBG("%s ", params[i]); \
            } \
        } \
    } while (0)

#define PRINT_NUMERIC_EVENT_PARAM \
    do { \
        DBG_FR("server: %s, event: %d, origin: %s", srv->name, event, origin); \
        DBG_FR("msg: %s", msg); \
        for (int i = 0; i < count; i++){ \
            if (i == 0) DBG_F("count: %d, params: [", count); \
            if (i == count - 1) { \
                DBG("%s]\n", params[i]); \
            } else { \
                DBG("%s ", params[i]); \
            } \
        } \
    } while (0)


#define CHECK_COUNT(x) \
    if (count < x) { \
        ERR_FR("Event '%s' except param count >= %d, actually %d", event, x, count); \
        return; \
    }

#define CHECK_NUMERIC_COUNT(x) \
    if (count < x) { \
        ERR_FR("Event '%3d' except param count >= %d, actually %d", event, x, count); \
        return; \
    }

void irc_event_connect(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);
    User *user = &srv->user;

    srv->stat = SERVER_CONNECTED;

    sui_add_sys_msgf(srv->ui, SYS_MSG_NORMAL, 0, _("Connected to %s(%s:%d)"),
            srv->name, srv->host, srv->port);

    sirc_cmd_nick(srv->irc, user->nick);
    sirc_cmd_user(srv->irc, user->username, "hostname", "servername", user->realname);
}

void irc_event_disconnect(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);

    srv->stat = SERVER_DISCONNECTED;

    sui_add_sys_msgf(srv->ui, SYS_MSG_NORMAL, 0, _("Disconnected from %s(%s:%d)"),
            srv->name, srv->host, srv->port);
}

void irc_event_ping(SircSession *sirc, const char *event){
    // Server *srv = sirc_get_ctx(sirc);
    // TODO: calc last ping time
}

void irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    // Server *srv = sirc_get_ctx(sirc);

    sirc_cmd_join(sirc, "#test");
    // sirc_cmd_join(sirc, "#srain");
    sirc_cmd_join(sirc, "#srain");
}

void irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    GList *lst;
    Chat *chat;
    User *user;


    const char *old_nick = origin;
    const char *new_nick = msg;

    snprintf(buf, sizeof(buf), _("%s is now known as %s"), old_nick, new_nick);

    lst = srv->chat_list;
    while (lst){
        chat = lst->data;
        // TODO: dialog support
        if ((user = chat_get_user(chat, origin)) != NULL){
            g_strlcpy(user->nick, new_nick, sizeof(user->nick));
            sui_ren_user(chat->ui, old_nick, new_nick, user->type);

            sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
            chat_log_log(srv->name, chat->name, buf);
        }
        lst = g_list_next(lst);
    }

    if (strncasecmp(origin, srv->user.nick, NICK_LEN) == 0){
        g_strlcpy(srv->user.nick, new_nick, sizeof(srv->user.nick));
    }
}

void irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    GList *lst;
    Chat *chat;
    User *user;

    PRINT_EVENT_PARAM;

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
            chat_rm_user(chat, user->nick);

            sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
            chat_log_log(srv->name, chat->name, buf);
        }
        lst = g_list_next(lst);
    }

    /* You quit */
    if (strncasecmp(origin, srv->user.nick, NICK_LEN) == 0){
        server_free(srv); // TODO ??
    }
}

void irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    Chat *chat;
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(0);
    g_return_if_fail(msg);

    const char *chan = msg;

    /* You has join a channel */
    if (strncasecmp(srv->user.nick, origin, NICK_LEN) == 0){
        server_add_chat(srv, chan, NULL);
    }

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);
    chat_add_user(chat, origin, USER_CHIGUA);

    snprintf(buf, sizeof(buf), _("%s has joined"), origin);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);
}

void irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *reason = msg;

    if (reason){
        snprintf(buf, sizeof(buf), _("%s has left: %s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%s has left"), origin);
    }

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);
    chat_rm_user(chat, origin);

    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);

    /* You has left a channel */
    if (strncasecmp(srv->user.nick, origin, NICK_LEN) == 0){
        server_rm_chat(srv, chan);
    }
}

void irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *mode = params[1];
    const char *mode_args = msg;

    snprintf(buf, sizeof(buf), _("mode %s %s %s by %s"),
            chan, mode, mode_args, origin);

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);

    if (mode[0] == '-'){
        // sui_ren_user(srv->name, chan, mode_args, mode_args, USER_CHIGUA);
        // TODO
    }
    else if (mode[0] == '+'){
        switch (mode[1]){
            case 'q':
                // sui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_OWNER);
                break;
            case 'a':
                // sui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_ADMIN);
                break;
            case 'o':
                // sui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_FULL_OP);
                break;
            case 'h':
                // sui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_HALF_OP);
                break;
            case 'v':
                // sui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_VOICED);
                break;
            default:
                break;
        }
    } else {
        ERR_FR("Wrong mode: %s. chan: %s, mode_args: %s",
                mode, chan, mode_args);
    }
}

void irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *mode = params[0];

    snprintf(buf, sizeof(buf), _("mode %s %s by %s"), origin, mode, origin);

    sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
    // TODO: How to log it?
    // chat_log_log(srv->name, chan, buf);
}

void irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *topic = msg;

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);
    sui_set_topic(chat->ui, topic);

    snprintf(buf, sizeof(buf), _("Topic for %s has changed to \"%s\""), chan, topic);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);
}

void irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *kick_nick = count >= 2 ? params[1] : ""; // TODO: ???
    const char *reason = msg;

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    if (strncasecmp(kick_nick, srv->user.nick, NICK_LEN) == 0){
        /* You are kicked 23333 */
        snprintf(buf, sizeof(buf), _("You are kicked by %s: %s"),
                kick_nick, reason);
        chat->joined = FALSE;
    } else {
        snprintf(buf, sizeof(buf), _("%s are kicked by %s: %s"),
                kick_nick, origin, reason);
    }

    sui_rm_user(chat->ui, kick_nick);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_ERROR, 0);
    chat_log_log(srv->name, chan, buf);
}

void irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);
    g_return_if_fail(msg);

    const char *chan = params[0];

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    chat_log_fmt(srv->name, chan, "<%s> %s", origin, msg);

    // TODO relay filter
    int flag = 0;
    flag |= strstr(msg, srv->user.nick) ? SRAIN_MSG_MENTIONED : 0;
    sui_add_recv_msg(chat->ui, origin, "", msg, flag);
}

void irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(0);
    g_return_if_fail(msg);

    chat = server_get_chat(srv, origin);
    g_return_if_fail(chat);

    chat_log_fmt(srv->name, origin, "<%s> %s", origin, msg);

    if (!filter_is_ignore(origin) && !filter_filter_check_message(NULL, origin, msg))
        sui_add_recv_msg(chat->ui, origin, "", msg, 0);
}

void irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);
    g_return_if_fail(msg);

    const char *target = params[0];
    chat = server_get_chat(srv, origin);
    if (strncasecmp(srv->user.nick, target, sizeof(srv->user.nick) - 1) == 0){
        /* NOTICE from user*/
        /* FIXME: Freenode specified :-(
         * This notice messaage is sent by Freenode's offical bot
         */
        SuiSession *ui = chat ? chat->ui : srv->ui;
        if (strcmp(origin, "NickServ") == 0
                || strcmp(origin, "ChanServ") == 0){
            sui_add_recv_msg(ui, origin, srv->name, msg, 0);
        } else {
            sui_add_recv_msg(ui, origin, "", msg, 0);
        }
    } else {
        /* NOTICE from channel */
        g_return_if_fail(chat);
        sui_add_recv_msg(chat->ui, origin, "", msg, 0);
    }

    chat_log_fmt(srv->name, target, "[%s] %s", origin, msg);
}

void irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    Chat *chat;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);
    g_return_if_fail(msg);

    // const char *chan = params[0];

}

void irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    CHECK_COUNT(0);
    const char *chan = msg;

    snprintf(buf, sizeof(buf), _("%s invites you into %s"), origin, chan);
    sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
    // TODO: Info bar

    // chat_log_log(srv->name, chan, buf);
}

void irc_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    Chat *chat;
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);
    g_return_if_fail(msg);

    const char *chan = params[0];

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);

    int flag = 0;
    flag |= strstr(buf, srv->user.nick) ? SRAIN_MSG_MENTIONED : 0;
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), origin, msg);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_ACTION, flag);
    chat_log_log(srv->name, chan, buf);
}

void irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_NUMERIC_EVENT_PARAM;

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
                int i = 1;
                GString *buf = g_string_new(NULL);

                while (i < count){
                    g_string_append_printf(buf, "%s ", params[i++]);
                }
                g_string_append_printf(buf, "%s", msg);

                sui_add_recv_msg(srv->ui, origin, srv->host, buf->str, 0);
                g_string_free(buf, TRUE);
                break;
            }
        case SIRC_RFC_ERR_NICKNAMEINUSE:
            {
                int len = strlen(srv->user.nick);
                if (len < sizeof(srv->user.nick) - 1){
                    srv->user.nick[len] = '_';
                    srv->user.nick[len + 1] = 0;
                    sirc_cmd_nick(srv->irc, srv->user.nick);
                } else {
                    ERR_FR("Your nickname is too long to add a trailing underline");
                }
                break;
            }

            /************************ NAMES message ************************/
        case SIRC_RFC_RPL_NAMREPLY:
            {
                CHECK_NUMERIC_COUNT(3);
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
                CHECK_NUMERIC_COUNT(2);

                const char *chan = params[1];
                const char *topic = msg;

                Chat *chat = server_get_chat(srv, chan);
                g_return_if_fail(chat);

                sui_set_topic(chat->ui, topic);
                break;
            }

            /************************ WHOIS message ************************/
        case SIRC_RFC_RPL_WHOISUSER:
            {
                CHECK_NUMERIC_COUNT(5);
                const char *nick = params[1];
                const char *user = params[2];
                const char *host = params[3];
                const char *realname = msg;
                snprintf(buf, sizeof(buf), "%s <%s@%s> %s",
                        nick, user, host, realname);
                sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
                // TODO pass NULL ui to send to current chat
                /* Use Real Name as avatar token :-( */
                plugin_avatar(nick, realname);
                break;
            }
        case SIRC_RFC_RPL_WHOISCHANNELS:
            {
                CHECK_NUMERIC_COUNT(3);
                snprintf(buf, sizeof(buf), _("%s is member of %s"), params[1], params[2]);
                sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
                break;
            }
        case SIRC_RFC_RPL_WHOISSERVER:
            CHECK_NUMERIC_COUNT(4);
            snprintf(buf, sizeof(buf), _("%s is attached to %s at \"%s\""),
                    params[1], params[2], params[3]);
            sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_WHOISIDLE:
            CHECK_NUMERIC_COUNT(4); // TODO
            snprintf(buf, sizeof(buf), _("%s is idle for %s seconds since %s"),
                    params[1], params[2], params[3]);
            sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_WHOWAS_TIME:
            CHECK_NUMERIC_COUNT(3);
            snprintf(buf, sizeof(buf), _("%s %s %s"),
                    params[1], msg, params[2]);
            sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_WHOISHOST:
        case SIRC_RFC_RPL_WHOISSECURE:
            CHECK_NUMERIC_COUNT(2);
            snprintf(buf, sizeof(buf), _("%s %s"), params[1], msg);
            sui_add_sys_msg(srv->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_ENDOFWHOIS:
            CHECK_NUMERIC_COUNT(2);
            sui_add_sys_msg(srv->ui, msg, SYS_MSG_NORMAL, 0);
            break;

            /************************ WHO message ************************/
        case SIRC_RFC_RPL_WHOREPLY:
            {
                CHECK_NUMERIC_COUNT(6);
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
        default:
            {
                // Error message
                if (event >= 400 && event < 600){
                    char buf[512];

                    snprintf(buf, sizeof(buf), _("ERROR[%3d]: %s"), event, msg);
                    sui_add_sys_msg(srv->ui, buf, SYS_MSG_ERROR, 0);
                    return;
                }

                int i;
                LOG_FR("Drop message, server: %s, event: %d, origin: %s, count: %u, params: [",
                        srv->name, event, origin, count);
                for (i = 0; i < count; i++) LOG("'%s', ", params[i]); LOG("]\n");
            }
    }
}
