/**
 * @file srv_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <strings.h>

#include "srv.h"
#include "srv_event.h"

#include "sirc_cmd.h"
#include "sirc_numeric.h"

#include "sui.h"

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
        DBG_F("count: %d, params: [", count); \
        for (int i = 0; i < count; i++){ \
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
        DBG_F("count: %d, params: [", count); \
        for (int i = 0; i < count; i++){ \
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

void srv_event_welcome(SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);

    sirc_cmd_join(sirc, "#test");
}

void srv_event_nick(SircSession *sirc, const char *event,
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
        strncpy(srv->user.nick, new_nick, NICK_LEN);
    }
}

void srv_event_quit(SircSession *sirc, const char *event,
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

void srv_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    const char *chan = msg;
    Chat *chat;

    /* You has join a channel */
    if (strncasecmp(srv->user.nick, origin, NICK_LEN) == 0){
        server_add_chat(srv, chan, NULL);
    }

    chat = server_get_chat(srv, chan);
    if (!chat) {
        ERR_FR("Received a JOIN message from a unjoined channel %s", chan);
    }
    chat_add_user(chat, origin);

    snprintf(buf, sizeof(buf), _("%s has joined"), origin);
    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);
}

void srv_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
    Chat *chat;
    User *user;

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
    chat_rm_user(chat, origin);

    sui_add_sys_msg(chat->ui, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);

    /* You has left a channel */
    if (strncasecmp(srv->user.nick, origin, NICK_LEN) == 0){
        server_rm_chat(srv, chan);
    }
}

void srv_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *mode = params[1];
    const char *mode_args = count >= 3 ? params[2] : "";

    snprintf(buf, sizeof(buf), _("mode %s %s %s by %s"),
            chan, mode, mode_args, origin);

    // srv_hdr_ui_sys_msg(srv->name, chan, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);

    if (mode[0] == '-'){
        // srv_hdr_ui_ren_user(srv->name, chan, mode_args, mode_args, USER_CHIGUA);
    }
    else if (mode[0] == '+'){
        switch (mode[1]){
            case 'q':
                // srv_hdr_ui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_OWNER);
                break;
            case 'a':
                // srv_hdr_ui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_ADMIN);
                break;
            case 'o':
                // srv_hdr_ui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_FULL_OP);
                break;
            case 'h':
                // srv_hdr_ui_ren_user(srv->name, chan, mode_args, mode_args,
                        // USER_HALF_OP);
                break;
            case 'v':
                // srv_hdr_ui_ren_user(srv->name, chan, mode_args, mode_args,
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

void srv_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *mode = params[0];

    snprintf(buf, sizeof(buf), _("mode %s %s by %s"), origin, mode, origin);

    // srv_hdr_ui_sys_msg(srv->name, "", buf, SYS_MSG_NORMAL, 0);
    // TODO: How to log it?
    // chat_log_log(srv->name, chan, buf);
}

void srv_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *topic = count >= 2 ? params[1] : "";

    // srv_hdr_ui_set_topic(srv->name, chan, topic);

    snprintf(buf, sizeof(buf), _("Topic for %s is \"%s\""), chan, topic);
    // srv_hdr_ui_sys_msg(srv->name, chan, buf, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, buf);
}

void srv_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *kick_nick = count >= 2 ? params[1] : "";;
    const char *reason = count >= 3 ? params[2] : "";

    snprintf(buf, sizeof(buf), _("%s are kicked from %s by %s: %s"),
            kick_nick, chan, origin, reason);
    // srv_hdr_ui_sys_msg(srv->name, chan, buf, SYS_MSG_ERROR, 0);
    chat_log_log(srv->name, chan, buf);

    // srv_hdr_ui_rm_user(srv->name, chan, kick_nick);

    /* You are kicked 23333 */
    if (strncasecmp(kick_nick, srv->user.nick, NICK_LEN) == 0){
        if (!chan){
            ERR_FR("Your are kicked from a channel which you haven't joined");
        }
       // srv_session_rm_chan(sess, chan);
    }
}

void srv_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *buf = count >= 2 ? params[1] : "";

    char* pure_msg = buf;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(buf);
    }

    chat_log_fmt(srv->name, chan, "<%s> %s", origin, pure_msg);

    char nick[NICK_LEN] = { 0 };

    filter_relaybot_trans(origin, nick, pure_msg);

    /* A message sent by relay bot */
    if (strlen(nick) > 0){
        if (!filter_is_ignore(nick) && !filter_filter_check_message(chan, nick, pure_msg)){
            if (!plugin_avatar_has_queried(nick)){
                // srv_session_who(sess, nick);
            }

            int flag = strstr(pure_msg, srv->user.nick) ? SRAIN_MSG_MENTIONED : 0;
            // srv_hdr_ui_recv_msg(srv->name, chan, nick, origin, pure_msg, flag);
        }
    } else {
        if (!filter_is_ignore(origin) && !filter_filter_check_message(chan, origin, pure_msg)){
            if (!plugin_avatar_has_queried(origin)) {
               // srv_session_who(sess, origin);
            }

            int flag = strstr(pure_msg, srv->user.nick) ? SRAIN_MSG_MENTIONED : 0;
            // srv_hdr_ui_recv_msg(srv->name, chan, origin, "", pure_msg, flag);
        }
    }

    free(pure_msg);
}

void srv_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *buf = count >= 2 ? params[1] : "";

    char* pure_msg = buf;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(buf);
    }

    chat_log_fmt(srv->name, origin, "<%s> %s", origin, pure_msg);

    if (!filter_is_ignore(origin) && !filter_filter_check_message(NULL, origin, pure_msg))
        // srv_hdr_ui_recv_msg(srv->name, origin, origin, "", pure_msg, 0);

    free(pure_msg);
}

void srv_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    // const char *nick = params[0];
    const char *buf = count >= 2 ? params[1] : "";

    char* pure_msg = buf;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(buf);
    }

    /* FIXME: Freenode specified :-(
     * This notice messaage is sent by Freenode's offical bot
     */
    if (strcmp(origin, "NickServ") == 0
            || strcmp(origin, "ChanServ") == 0){
        // srv_hdr_ui_recv_msg(srv->name, origin, origin, srv->name, pure_msg, 0);
    } else {
        // srv_hdr_ui_recv_msg(srv->name, origin, origin, "", pure_msg, 0);
    }
    chat_log_fmt(srv->name, origin, "[%s] %s", origin, pure_msg);

    free(pure_msg);
}

void srv_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *buf = count >= 2 ? params[1] : "";

    char* pure_msg = buf;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(buf);
    }

    // srv_hdr_ui_recv_msg(srv->name, chan, origin, "", pure_msg, 0);
    chat_log_fmt(srv->name, chan, "[%s] %s", origin, pure_msg);

    free(pure_msg);
}

void srv_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    CHECK_COUNT(1);
    const char *chan = count >= 2 ? params[1] : "";

    snprintf(buf, sizeof(buf), _("%s invites you into %s"), origin, chan);
    // srv_hdr_ui_sys_msg(srv->name, "", buf, SYS_MSG_NORMAL, 0);

    // TODO: How to log it?
    // chat_log_log(srv->name, chan, buf);
    // INFO BAR
}

void srv_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;

    CHECK_COUNT(1);
    const char *chan = params[0];

    char* pure_msg = msg; // TODO strip
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(msg);
    }

    int flag = strstr(buf, srv->user.nick) ? SRAIN_MSG_MENTIONED : 0;
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), origin, pure_msg);
    // srv_hdr_ui_sys_msg(srv->name, chan, buf, SYS_MSG_ACTION, flag);
    chat_log_log(srv->name, chan, buf);

    free(pure_msg);
}

void srv_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count, const char *msg){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_NUMERIC_EVENT_PARAM;

    switch (event) {
        case RPL_WELCOME:
        case RPL_YOURHOST:
        case RPL_CREATED:
        case RPL_MOTDSTART:
        case RPL_MOTD:
        case RPL_ENDOFMOTD:
        case RPL_MYINFO:
        case RPL_BOUNCE:
        case RPL_LUSEROP:
        case RPL_LUSERUNKNOWN:
        case RPL_LUSERCHANNELS:
        case RPL_LUSERCLIENT:
        case RPL_LUSERME:
        case RPL_ADMINME:
        case RPL_STATSDLINE:
        case RPL_LOCALUSERS:
        case RPL_GLOBALUSERS:
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
        default:
            break;
    }
}

void srv_event_connect(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);
    User *user = &srv->user;

    srv->stat = SERVER_CONNECTED;

    sui_add_sys_msgf(srv->ui, SYS_MSG_NORMAL, 0, _("Connected to %s(%s:%d)"),
            srv->name, srv->host, srv->port);

    sirc_cmd_nick(srv->irc, user->nick);
    sirc_cmd_user(srv->irc, user->username, "hostname", "servername", user->realname);
}

void srv_event_disconnect(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);

    srv->stat = SERVER_DISCONNECTED;

    sui_add_sys_msgf(srv->ui, SYS_MSG_NORMAL, 0, _("Disconnected from %s(%s:%d)"),
            srv->name, srv->host, srv->port);
}

void srv_event_ping(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);
    // TODO: calc last ping time
}
