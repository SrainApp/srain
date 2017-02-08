/**
 * @file srv_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

// #define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <strings.h>

#include "srv.h"
#include "srv_event.h"

#include "ui.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "filter.h"
#include "meta.h"
#include "chat_log.h"
#include "plugin.h"

#define __DBG_ON
#define __LOG_ON

#define PRINT_EVENT_PARAM \
    do { \
        DBG_F("server: %s, event: %s, origin: %s, count %u, params: [", \
                srv->name, event, origin, count); \
        int i; \
        for (i = 0; i < count; i++) DBG("'%s', ", params[i]); \
        DBG("]\n"); \
    } while (0)

#define CHECK_COUNT(x) \
    if (count < x) { \
        ERR_FR("server: %s, count: %u, except >= %u", srv->name, count, x); \
        return; \
    }

void srv_event_welcome(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    GList *chan_list;

    PRINT_EVENT_PARAM;

    // sess->stat = SRV_SESSION_STAT_CONNECT;

    /*
    chan_list = sess->chan_list;
    while (chan_list){
        chan = chan_list->data;
       // srv_session_join(sess, chan->name, NULL);
        chan_list = g_list_next(chan_list);
    }
    */
    LOG_FR("Welcome");
}

void srv_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];
    GList *lst;
    Channel *chan;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *new_nick = params[0];
    lst = srv->chan_list;
    while (lst){
        chan = lst->data;
        // TODO
        /*
        if (ui_ren_user(srv->name, chan->name, origin, new_nick, 0) == SRN_OK) {
            snprintf(msg, sizeof(msg), _("%s is now known as %s"), origin, new_nick);
            ui_sys_msg(srv->name, chan->name, msg, SYS_MSG_NORMAL, 0);
        }

            chat_log_log(srv->name, chan->name, msg);
            */
        lst = g_list_next(lst);
    }

    if (strncasecmp(origin, srv->user.nick, NICK_LEN) == 0){
        strncpy(srv->user.nick, new_nick, NICK_LEN);
    }
}

void srv_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];
    GList *chan_list;

    PRINT_EVENT_PARAM;
    CHECK_COUNT(0);

    const char *reason = count >= 1 ? params[0] : "";

    snprintf(msg, sizeof(msg), _("%s has quit: %s"), origin, reason);

    /*
    chan_list = sess->chan_list;
    while (chan_list){
        chan = chan_list->data;
        if (srv_session_user_exist(sess, chan->name, origin)){
           // srv_session_rm_user(sess, chan->name, origin);

            // srv_hdr_ui_rm_user(srv->name, chan->name, origin);
            // srv_hdr_ui_sys_msg(srv->name, chan->name, msg, SYS_MSG_NORMAL, 0);

            chat_log_log(srv->name, chan->name, msg);
        }
        chan_list = g_list_next(chan_list);
    }
    */

    /* You quit */
    if (strncasecmp(origin, srv->user.nick, NICK_LEN) == 0){
        LOG_FR("session: %s, origin: %s, reason: %s", srv->name, origin, reason);
        /* Remove all chans belong to this session */
        // srv_hdr_ui_rm_chat(srv->name, "");
        /*
        sess->stat = SRV_SESSION_STAT_CLOSE;
       // srv_session_free(sess);
        */
    }
}

void srv_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];
    const char *chan = params[0];

    /* You has join a channel */
    if (strncasecmp(srv->user.nick, origin, NICK_LEN) == 0){
        ui_add_chat(srv, chan, CHAT_CHANNEL);

        //// srv_session_add_chan(sess, chan);
    }

    /* SRV user set */
   // srv_session_add_user(sess, chan, origin);

    /* UI user list */
    // srv_hdr_ui_add_user(srv->name, chan, origin, USER_CHIGUA);

    snprintf(msg, sizeof(msg), _("%s has joined %s"), origin, chan);
    // srv_hdr_ui_sys_msg(srv->name, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, msg);
}

void srv_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *reason = count >= 2 ? params[1] : "";;

    // srv_hdr_ui_rm_user(srv->name, chan, origin);

   // srv_session_rm_user(sess, chan, origin);

    snprintf(msg, sizeof(msg), _("%s has left %s: %s"), origin, chan, reason);
    // srv_hdr_ui_sys_msg(srv->name, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, msg);

    /* You has left a channel */
    if (strncasecmp(srv->user.nick, origin, NICK_LEN) == 0){
        // srv_hdr_ui_rm_chat(srv->name, chan);
       // srv_session_rm_chan(sess, chan);
    }
}

void srv_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *mode = params[1];
    const char *mode_args = count >= 3 ? params[2] : "";

    snprintf(msg, sizeof(msg), _("mode %s %s %s by %s"),
            chan, mode, mode_args, origin);

    // srv_hdr_ui_sys_msg(srv->name, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, msg);

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
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *mode = params[0];

    snprintf(msg, sizeof(msg), _("mode %s %s by %s"), origin, mode, origin);

    // srv_hdr_ui_sys_msg(srv->name, "", msg, SYS_MSG_NORMAL, 0);
    // TODO: How to log it?
    // chat_log_log(srv->name, chan, msg);
}

void srv_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *topic = count >= 2 ? params[1] : "";

    // srv_hdr_ui_set_topic(srv->name, chan, topic);

    snprintf(msg, sizeof(msg), _("Topic for %s is \"%s\""), chan, topic);
    // srv_hdr_ui_sys_msg(srv->name, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(srv->name, chan, msg);
}

void srv_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *kick_nick = count >= 2 ? params[1] : "";;
    const char *reason = count >= 3 ? params[2] : "";

    snprintf(msg, sizeof(msg), _("%s are kicked from %s by %s: %s"),
            kick_nick, chan, origin, reason);
    // srv_hdr_ui_sys_msg(srv->name, chan, msg, SYS_MSG_ERROR, 0);
    chat_log_log(srv->name, chan, msg);

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
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *msg = count >= 2 ? params[1] : "";

    char* pure_msg = msg;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(msg);
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
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *msg = count >= 2 ? params[1] : "";

    char* pure_msg = msg;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(msg);
    }

    chat_log_fmt(srv->name, origin, "<%s> %s", origin, pure_msg);

    if (!filter_is_ignore(origin) && !filter_filter_check_message(NULL, origin, pure_msg))
        // srv_hdr_ui_recv_msg(srv->name, origin, origin, "", pure_msg, 0);

    free(pure_msg);
}

void srv_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    // const char *nick = params[0];
    const char *msg = count >= 2 ? params[1] : "";

    char* pure_msg = msg;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(msg);
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
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *msg = count >= 2 ? params[1] : "";

    char* pure_msg = msg;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(msg);
    }

    // srv_hdr_ui_recv_msg(srv->name, chan, origin, "", pure_msg, 0);
    chat_log_fmt(srv->name, chan, "[%s] %s", origin, pure_msg);

    free(pure_msg);
}

void srv_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char msg[512];

    CHECK_COUNT(1);
    const char *chan = count >= 2 ? params[1] : "";

    snprintf(msg, sizeof(msg), _("%s invites you into %s"), origin, chan);
    // srv_hdr_ui_sys_msg(srv->name, "", msg, SYS_MSG_NORMAL, 0);

    // TODO: How to log it?
    // chat_log_log(srv->name, chan, msg);
}

void srv_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];

    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    const char *chan = params[0];
    const char *msg = params[1];

    char* pure_msg = msg;
    if (!pure_msg){
        ERR_FR("Failed to strip color from irc message");
        pure_msg = strdup(msg);
    }

    int flag = strstr(msg, srv->user.nick) ? SRAIN_MSG_MENTIONED : 0;
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), origin, pure_msg);
    // srv_hdr_ui_sys_msg(srv->name, chan, buf, SYS_MSG_ACTION, flag);
    chat_log_log(srv->name, chan, buf);

    free(pure_msg);
}

void srv_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count){
    Server *srv = sirc_get_ctx(sirc);
    char buf[512];
}

void srv_event_connect(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);
    User *user = &srv->user;

    LOG_FR("Server %s connected", srv->name);

    sirc_cmd_nick(srv->irc, user->nick);
    sirc_cmd_user(srv->irc, user->username, "hostname", "servername", user->realname);
}

void srv_event_disconnect(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);

    LOG_FR("Server %s disconnected", srv->name);
}

void srv_event_ping(SircSession *sirc, const char *event){
    Server *srv = sirc_get_ctx(sirc);

    LOG_FR("Server %s ping", srv->name);
}
