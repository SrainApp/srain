/**
 * @file server_irc_event.c
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

#include "server.h"
#include "server_irc_event.h"

#include "sui/sui.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "filter.h"
#include "meta.h"
#include "chat_log.h"
#include "plugin.h"
#include "decorator.h"

void server_irc_event_connect(SircSession *sirc, const char *event){
    Server *srv;
    User *user;

    g_return_if_fail(srv = sirc_get_ctx(sirc));

    srv->stat = SERVER_CONNECTED;
    user = srv->user;

    sui_add_sys_msgf(srv->chat->ui, SYS_MSG_NORMAL, 0, _("Connected to %s(%s:%d)"),
            srv->info->name, srv->info->host, srv->info->port);

    sirc_cmd_nick(srv->irc, user->nick);
    sirc_cmd_user(srv->irc, user->username, "hostname", "servername", user->realname);
}

void server_irc_event_disconnect(SircSession *sirc, const char *event){
    Server *srv;

    g_return_if_fail(srv = sirc_get_ctx(sirc));

    srv->stat = SERVER_DISCONNECTED;

    sui_add_sys_msgf(srv->chat->ui, SYS_MSG_NORMAL, 0, _("Disconnected from %s(%s:%d)"),
            srv->info->name, srv->info->host, srv->info->port);
}

void server_irc_event_ping(SircSession *sirc, const char *event){
    Server *srv;

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    // TODO: calc last ping time
}

void server_irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;

    g_return_if_fail(srv = sirc_get_ctx(sirc));
}

void server_irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    GSList *lst;
    Server *srv;
    Chat *chat;
    User *user;

    g_return_if_fail(srv = sirc_get_ctx(sirc));

    const char *old_nick = origin;
    const char *new_nick = msg;

    snprintf(buf, sizeof(buf), _("%s is now known as %s"), old_nick, new_nick);

    lst = srv->chat_list;
    while (lst){
        chat = lst->data;
        // TODO: dialog nick track support
        if ((user = chat_get_user(chat, old_nick)) != NULL){
            user_rename(user, new_nick);
            sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
            chat_log_log(srv->info->name, chat->name, buf);
        }
        lst = g_slist_next(lst);
    }

    if (sirc_nick_cmp(old_nick, srv->user->nick)){
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

    g_return_if_fail(srv = sirc_get_ctx(sirc));

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
            chat_log_log(srv->info->name, chat->name, buf);
        }
        lst = g_slist_next(lst);
    }

    /* You quit */
    if (sirc_nick_cmp(origin, srv->user->nick)){
        server_disconnect(srv);
        server_free(srv);
    }
}

void server_irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    bool youjoin;
    char buf[512];
    Server *srv;
    Chat *chat;

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(msg || count >= 1);

    const char *chan = msg ? msg : params[0];

    /* You has join a channel */
    youjoin = sirc_nick_cmp(srv->user->nick, origin);

    if (youjoin) {
        server_add_chat(srv, chan);
    }

    g_return_if_fail(chat = server_get_chat(srv, chan));

    if (youjoin) {
        chat->joined = TRUE;
    }

    chat_add_user(chat, origin, USER_CHIGUA);

    if (youjoin) {
        snprintf(buf, sizeof(buf), _("You has joined"));
    } else {
        snprintf(buf, sizeof(buf), _("%s has joined"), origin);
    }

    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->info->name, chan, buf);
}

void server_irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;
    Chat *chat;

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(count >= 1);

    const char *chan = params[0];
    const char *reason = msg;

    if (reason){
        snprintf(buf, sizeof(buf), _("%s has left: %s"), origin, reason);
    } else {
        snprintf(buf, sizeof(buf), _("%s has left"), origin);
    }

    g_return_if_fail(chat = server_get_chat(srv, chan));

    chat_rm_user(chat, origin);

    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->info->name, chan, buf);

    /* You has left a channel */
    if (strncasecmp(srv->user->nick, origin, NICK_LEN) == 0){
        chat->joined = FALSE;
        server_rm_chat(srv, chan);
    }
}

void server_irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    GString *buf;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count >= 1);

    const char *chan = params[0];

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(chat = server_get_chat(srv, chan));

    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %s "), chan);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(buf, "%s ", params[i]);
    }

    if (msg) {
        g_string_append_printf(buf, "%s ", msg);
    }

    g_string_append_printf(buf, _("by %s"), origin);

    sui_add_sys_msg(chat->ui, buf->str, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->info->name, chan, buf->str);

    /* TODO: parse more modes */
    /*
    */

    g_string_free(buf, TRUE);
}

void server_irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    GString *buf;
    Server *srv;

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(count >= 1);

    const char *nick = params[0];

    buf = g_string_new(NULL);
    g_string_printf(buf, _("mode %s "), nick);
    for (int i = 1; i < count; i++) {
        g_string_append_printf(buf, "%s ", params[i]);
    }

    if (msg) {
        g_string_append_printf(buf, "%s ", msg);
    }

    g_string_append_printf(buf, _("by %s"), origin);

    sui_add_sys_msg(srv->chat->ui, buf->str, SYS_MSG_NORMAL, 0);
    // TODO: How to log it?
    // chat_log_log(srv->info->name, chan, buf);

    g_string_free(buf, TRUE);
}

void server_irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;
    Chat *chat;

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(count >= 1);

    const char *chan = params[0];
    const char *topic = msg;

    chat = server_get_chat(srv, chan);
    g_return_if_fail(chat);
    sui_set_topic(chat->ui, topic);

    snprintf(buf, sizeof(buf), _("Topic for %s has changed to \"%s\""), chan, topic);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->info->name, chan, buf);
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

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(chat = server_get_chat(srv, chan));

    if (strncasecmp(kick_nick, srv->user->nick, NICK_LEN) == 0){
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
    chat_log_log(srv->info->name, chan, buf);
}

void server_irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;
    User *user;
    Message *message;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);
    const char *chan = params[0];

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(chat = server_get_chat(srv, chan));

    user = chat_get_user(chat, origin);
    LOG_FR("%s %s", origin, user->nick);
    message = message_new(chat, user, msg);

    decorate_message(message,
            DECORATOR_BOT2HUMAN | DECORATOR_MIRC_STRIP | DECORATOR_PANGO_MARKUP,
            NULL);

    chat_log_fmt(srv->info->name, chan, "<%s> %s", origin, msg);

    sui_add_recv_msg(chat->ui, message->dname, "", message->dcontent, 0);

    // message_free(message)            ;
}

void server_irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 0);
    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(chat = server_get_chat(srv, origin));

    chat_log_fmt(srv->info->name, origin, "<%s> %s", origin, msg);

    // if (!filter_is_ignore(origin) && !filter_filter_check_message(NULL, origin, msg))
        sui_add_recv_msg(chat->ui, origin, "", msg, 0);
}

void server_irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);
    const char *target = params[0];

    g_return_if_fail(srv = sirc_get_ctx(sirc));

    chat = server_get_chat(srv, target);

    SuiSession *ui = chat ? chat->ui : srv->chat->ui;

    if (strcmp(origin, "NickServ") == 0
            || strcmp(origin, "ChanServ") == 0){
        /* FIXME: Freenode specified :-(
         * This notice messaage is sent by Freenode's offical bot
         */
        sui_add_recv_msg(ui, origin, srv->info->name, msg, 0);
    } else {
        sui_add_recv_msg(ui, origin, "", msg, 0);
    }

    chat_log_fmt(srv->info->name, target, "[%s] %s", origin, msg);
}

void server_irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);
    const char *chan = params[0];

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(chat = server_get_chat(srv, chan));

    sui_add_recv_msg(chat->ui, origin, "", msg, 0);
}

void server_irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;

    g_return_if_fail(count >= 0);
    const char *chan = msg;

    g_return_if_fail(srv = sirc_get_ctx(sirc));

    snprintf(buf, sizeof(buf), _("%s invites you into %s"), origin, chan);
    sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
    // TODO: Info bar

    // chat_log_log(srv->info->name, chan, buf);
}

void server_irc_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;
    Chat *chat;

    g_return_if_fail(msg);
    g_return_if_fail(count >= 1);
    const char *chan = params[0];

    g_return_if_fail(srv = sirc_get_ctx(sirc));
    g_return_if_fail(chat = server_get_chat(srv, chan));

    int flag = 0;
    flag |= strstr(buf, srv->user->nick) ? SRAIN_MSG_MENTIONED : 0;
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), origin, msg);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_ACTION, flag);
    chat_log_log(srv->info->name, chan, buf);
}

void server_irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    char buf[512];
    Server *srv;

    g_return_if_fail(srv = sirc_get_ctx(sirc));

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
                GString *buf;
                char *dcontent;

                buf = g_string_new(NULL);
                while (i < count){
                    g_string_append_printf(buf, "%s ", params[i++]);
                }
                if (msg) g_string_append_printf(buf, "%s", msg);

                dcontent = decorate_content(buf->str, DECORATOR_MIRC_STRIP | DECORATOR_PANGO_MARKUP);
                if (dcontent){
                    sui_add_recv_msg(srv->chat->ui, origin, srv->info->host, dcontent, 0);
                    g_free(dcontent);
                }

                g_string_free(buf, TRUE);

                break;
            }
        case SIRC_RFC_ERR_NICKNAMEINUSE:
            {
                int len = strlen(srv->user->nick);
                if (len < sizeof(srv->user->nick) - 1){
                    srv->user->nick[len] = '_';
                    srv->user->nick[len + 1] = 0;
                    sirc_cmd_nick(srv->irc, srv->user->nick);
                } else {
                    ERR_FR("Your nickname is too long to add a trailing underline");
                }
                break;
            }

            /************************ NAMES message ************************/
        case SIRC_RFC_RPL_NAMREPLY:
            {
                g_return_if_fail(count >= 3);
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
                g_return_if_fail(count >= 5);
                const char *nick = params[1];
                const char *user = params[2];
                const char *host = params[3];
                const char *realname = msg;
                snprintf(buf, sizeof(buf), "%s <%s@%s> %s",
                        nick, user, host, realname);
                sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
                // TODO pass NULL ui to send to current chat
                /* Use Real Name as avatar token :-( */
                plugin_avatar(nick, realname);
                break;
            }
        case SIRC_RFC_RPL_WHOISCHANNELS:
            {
                g_return_if_fail(count >= 3);
                snprintf(buf, sizeof(buf), _("%s is member of %s"), params[1], params[2]);
                sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
                break;
            }
        case SIRC_RFC_RPL_WHOISSERVER:
            g_return_if_fail(count >= 4);
            snprintf(buf, sizeof(buf), _("%s is attached to %s at \"%s\""),
                    params[1], params[2], params[3]);
            sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_WHOISIDLE:
            g_return_if_fail(count >= 4); // TODO
            snprintf(buf, sizeof(buf), _("%s is idle for %s seconds since %s"),
                    params[1], params[2], params[3]);
            sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_WHOWAS_TIME:
            g_return_if_fail(count >= 3);
            snprintf(buf, sizeof(buf), _("%s %s %s"),
                    params[1], msg, params[2]);
            sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_WHOISHOST:
        case SIRC_RFC_RPL_WHOISSECURE:
            g_return_if_fail(count >= 2);
            snprintf(buf, sizeof(buf), _("%s %s"), params[1], msg);
            sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_NORMAL, 0);
            break;
        case SIRC_RFC_RPL_ENDOFWHOIS:
            g_return_if_fail(count >= 2);
            sui_add_sys_msg(srv->chat->ui, msg, SYS_MSG_NORMAL, 0);
            break;

            /************************ WHO message ************************/
        case SIRC_RFC_RPL_WHOREPLY:
            {
                g_return_if_fail(count >= 6);
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
                    sui_add_sys_msg(srv->chat->ui, buf, SYS_MSG_ERROR, 0);
                    return;
                }

                int i;
                LOG_FR("Drop message, server: %s, event: %d, origin: %s, count: %u, params: [",
                        srv->info->name, event, origin, count);
                for (i = 0; i < count; i++) LOG("'%s', ", params[i]); LOG("]\n");
            }
    }
}
