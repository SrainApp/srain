/**
 * @file srv_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

// #define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <strings.h>

#include <libircclient.h>
#include <libirc_rfcnumeric.h>
#include "libircclient_ex.h"

#include "srv_session.h"
#include "srv_event.h"
#include "srv_hdr.h"

#include "i18n.h"
#include "log.h"
#include "filter.h"
#include "meta.h"
#include "chat_log.h"
#include "plugin.h"

#define PRINT_EVENT_PARAM \
    do { \
        DBG_F("session: %s, event: %s, origin: %s, count %u, params: [", \
                sess->host, event, origin, count); \
        int i; \
        for (i = 0; i < count; i++) DBG("'%s', ", params[i]); \
        DBG("]\n"); \
    } while (0)

#define CHECK_COUNT(x) \
    if (count < x) { \
        ERR_FR("sessions: %s, count: %u, except >= %u", sess->host, count, x); \
        return; \
    }

/* TODO: remove me strip unprintable char and irc color code */
static void strip(char *str){
    int i;
    int j;
    int len;

    j = 0;
    len = strlen(str);

    for (i = 0; i < len; i++){
        switch (str[i]){
            case 2: case 0xf: case 0x16:
            case 0x1d: case 0x1f:
                break;
            case 3:  // irc color code
                if (str[i+1] >= '0' && str[i+1] <= '9'){
                    if (str[i+2] >= '0' && str[i+2] <= '9'){
                        i += 2;
                    } else {
                        i += 1;
                    }
                }
                break;
            default:
                str[j++] = str[i];
        }
    }

    str[j] = '\0';
}

void srv_event_connect(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    GList *chan_list;
    channel_t *chan;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    sess->stat = SESS_CONNECT;

    LOG_FR("Connected to %s", sess->host);

    chan_list = sess->chan_list;
    while (chan_list){
        chan = chan_list->data;
        srv_session_join(sess, chan->name, NULL);
        chan_list = g_list_next(chan_list);
    }
}

void srv_event_nick(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;
    channel_t *chan;
    GList *chan_list;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *new_nick = params[0];

    snprintf(msg, sizeof(msg), _("%s is now known as %s"), origin, new_nick);

    chan_list = sess->chan_list;
    while (chan_list){
        chan = chan_list->data;
        if (srv_session_user_exist(sess, chan->name, origin)){
            srv_session_rm_user(sess, chan->name, origin);
            srv_session_add_user(sess, chan->name, new_nick);

            srv_hdr_ui_ren_user(sess->host, chan->name, origin, new_nick, 0);
            srv_hdr_ui_sys_msg(sess->host, chan->name, msg, SYS_MSG_NORMAL, 0);

            chat_log_log(sess->host, chan->name, msg);
        }
        chan_list = g_list_next(chan_list);
    }

    if (strncasecmp(origin, sess->nickname, NICK_LEN) == 0){
        strncpy(sess->nickname, new_nick, NICK_LEN);
    }
}

void srv_event_quit(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;
    channel_t *chan;
    GList *chan_list;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(0);

    const char *reason = count >= 1 ? params[0] : "";

    snprintf(msg, sizeof(msg), _("%s has quit: %s"), origin, reason);

    chan_list = sess->chan_list;
    while (chan_list){
        chan = chan_list->data;
        if (srv_session_user_exist(sess, chan->name, origin)){
            srv_session_rm_user(sess, chan->name, origin);

            srv_hdr_ui_rm_user(sess->host, chan->name, origin);
            srv_hdr_ui_sys_msg(sess->host, chan->name, msg, SYS_MSG_NORMAL, 0);

            chat_log_log(sess->host, chan->name, msg);
        }
        chan_list = g_list_next(chan_list);
    }

    /* You quit */
    if (strncasecmp(origin, sess->nickname, NICK_LEN) == 0){
        LOG_FR("session: %s, origin: %s, reason: %s", sess->host, origin, reason);
        /* Remove all chans belong to this session */
        srv_hdr_ui_rm_chat(sess->host, "");
        sess->stat = SESS_CLOSE;
        srv_session_free(sess);
    }
}

void srv_event_join(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];

    /* YOU has join a channel */
    if (strncasecmp(sess->nickname, origin, NICK_LEN) == 0){
        srv_hdr_ui_add_chat(sess->host, chan, origin, CHAT_CHANNEL);

        srv_session_add_chan(sess, chan);
    }

    /* SRV user set */
    srv_session_add_user(sess, chan, origin);

    /* UI user list */
    srv_hdr_ui_add_user(sess->host, chan, origin, USER_CHIGUA);

    snprintf(msg, sizeof(msg), _("%s has joined %s"), origin, chan);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(sess->host, chan, msg);
}

void srv_event_part(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *reason = count >= 2 ? params[1] : "";;

    srv_hdr_ui_rm_user(sess->host, chan, origin);

    srv_session_rm_user(sess, chan, origin);

    snprintf(msg, sizeof(msg), _("%s has left %s: %s"), origin, chan, reason);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(sess->host, chan, msg);

    /* YOU has left a channel */
    if (strncasecmp(sess->nickname, origin, NICK_LEN) == 0){
        srv_hdr_ui_rm_chat(sess->host, chan);
        srv_session_rm_chan(sess, chan);
    }
}

void srv_event_mode(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    const char *mode = params[1];
    const char *mode_args = count >= 3 ? params[2] : "";

    snprintf(msg, sizeof(msg), _("mode %s %s %s by %s"),
            chan, mode, mode_args, origin);

    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(sess->host, chan, msg);

    if (mode[0] == '-'){
        srv_hdr_ui_ren_user(sess->host, chan, mode_args, mode_args, USER_CHIGUA);
    }
    else if (mode[0] == '+'){
        switch (mode[1]){
            case 'q':
                srv_hdr_ui_ren_user(sess->host, chan, mode_args, mode_args,
                        USER_OWNER);
                break;
            case 'a':
                srv_hdr_ui_ren_user(sess->host, chan, mode_args, mode_args,
                        USER_ADMIN);
                break;
            case 'o':
                srv_hdr_ui_ren_user(sess->host, chan, mode_args, mode_args,
                        USER_FULL_OP);
                break;
            case 'h':
                srv_hdr_ui_ren_user(sess->host, chan, mode_args, mode_args,
                        USER_HALF_OP);
                break;
            case 'v':
                srv_hdr_ui_ren_user(sess->host, chan, mode_args, mode_args,
                        USER_VOICED);
                break;
            default:
                break;
        }
    } else {
        ERR_FR("Wrong mode: %s. chan: %s, mode_args: %s",
                mode, chan, mode_args);
    }
}

void srv_event_umode(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *mode = params[0];

    snprintf(msg, sizeof(msg), _("mode %s %s by %s"), origin, mode, origin);

    srv_hdr_ui_sys_msg(sess->host, "", msg, SYS_MSG_NORMAL, 0);
    // TODO: How to log it?
    // chat_log_log(sess->host, chan, msg);
}

void srv_event_topic(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *topic = count >= 2 ? params[1] : "";

    srv_hdr_ui_set_topic(sess->host, chan, topic);

    snprintf(msg, sizeof(msg), _("Topic for %s is \"%s\""), chan, topic);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL, 0);
    chat_log_log(sess->host, chan, msg);
}

void srv_event_kick(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *kick_nick = count >= 2 ? params[1] : "";;
    const char *reason = count >= 3 ? params[2] : "";

    snprintf(msg, sizeof(msg), _("%s are kicked from %s by %s: %s"),
            kick_nick, chan, origin, reason);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_ERROR, 0);
    chat_log_log(sess->host, chan, msg);

    srv_hdr_ui_rm_user(sess->host, chan, kick_nick);
}

void srv_event_channel(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char vmsg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    const char *chan = params[0];
    const char *msg = count >= 2 ? params[1] : "";
    strncpy(vmsg, msg, sizeof(vmsg));

    strip(vmsg);

    chat_log_fmt(sess->host, chan, "<%s> %s", origin, vmsg);

    char nick[NICK_LEN] = { 0 };
    filter_relaybot_trans(origin, nick, vmsg);

    /* A message sent by relay bot */
    if (strlen(nick) > 0){
        if (!filter_is_ignore(nick)){
            if (!plugin_avatar_has_queried(nick)) srv_session_who(sess, nick);

            int flag = strstr(vmsg, sess->nickname) ? SRAIN_MSG_MENTIONED : 0;
            srv_hdr_ui_recv_msg(sess->host, chan, nick, origin, vmsg, flag);
        }
    } else {
        if (!filter_is_ignore(origin)){
            if (!plugin_avatar_has_queried(origin))
                srv_session_who(sess, origin);

            int flag = strstr(vmsg, sess->nickname) ? SRAIN_MSG_MENTIONED : 0;
            srv_hdr_ui_recv_msg(sess->host, chan, origin, "", vmsg, flag);
        }
    }
}

void srv_event_privmsg(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    char *msg = strdup(count >= 2 ? params[1] : "");

    strip(msg);

    chat_log_fmt(sess->host, origin, "<%s> %s", origin, msg);

    if (!filter_is_ignore(origin))
        srv_hdr_ui_recv_msg(sess->host, origin, origin, "", msg, 0);

    free(msg);
}

void srv_event_notice(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);

    // const char *nick = params[0];
    char *msg = strdup(count >= 2 ? params[1] : "");

    strip(msg);

    /* FIXME: Freenode specified :-(
     * This notice messaage is sent by Freenode's offical bot
     */
    if (strcmp(origin, "NickServ") == 0
            || strcmp(origin, "ChanServ") == 0){
        srv_hdr_ui_recv_msg(sess->host, origin, origin, sess->host, msg, 0);
    } else {
        srv_hdr_ui_recv_msg(sess->host, origin, origin, "", msg, 0);
    }
    chat_log_fmt(sess->host, origin, "[%s] %s", origin, msg);

    free(msg);
}

void srv_event_channel_notice(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);

    const char *chan = params[0];
    char *msg = strdup(count >= 2 ? params[1] : "");

    strip(msg);

    srv_hdr_ui_recv_msg(sess->host, chan, origin, "", msg, 0);
    chat_log_fmt(sess->host, chan, "[%s] %s", origin, msg);

    free(msg);
}

void srv_event_invite(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    PRINT_EVENT_PARAM;

    CHECK_COUNT(1);
    const char *chan = count >= 2 ? params[1] : "";

    snprintf(msg, sizeof(msg), _("%s invites you into %s"), origin, chan);
    srv_hdr_ui_sys_msg(sess->host, "", msg, SYS_MSG_NORMAL, 0);

    // TODO: How to log it?
    // chat_log_log(sess->host, chan, msg);
}

void srv_event_ctcp_action(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char buf[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    const char *chan = params[0];
    char *msg = strdup(params[1]);

    strip(msg);

    int flag = strstr(msg, sess->nickname) ? SRAIN_MSG_MENTIONED : 0;
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), origin, msg);
    srv_hdr_ui_sys_msg(sess->host, chan, buf, SYS_MSG_ACTION, flag);
    chat_log_log(sess->host, chan, buf);

    free(msg);
}

void srv_event_numeric (irc_session_t *irc_session, unsigned int event,
        const char *origin, const char **params, unsigned int count){
    char buf[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    switch (event){
            /************************ Server Message ************************/
        case LIBIRC_RFC_RPL_WELCOME:
        case LIBIRC_RFC_RPL_YOURHOST:
        case LIBIRC_RFC_RPL_CREATED:
        case LIBIRC_RFC_RPL_MOTDSTART:
        case LIBIRC_RFC_RPL_MOTD:
        case LIBIRC_RFC_RPL_ENDOFMOTD:
        case LIBIRC_RFC_RPL_MYINFO:
        case LIBIRC_RFC_RPL_BOUNCE:
        case LIBIRC_RFC_RPL_LUSEROP:
        case LIBIRC_RFC_RPL_LUSERUNKNOWN:
        case LIBIRC_RFC_RPL_LUSERCHANNELS:
        case LIBIRC_RFC_RPL_LUSERCLIENT:
        case LIBIRC_RFC_RPL_LUSERME:
        case LIBIRC_RFC_RPL_ADMINME:
        case NORFC1459_RPL_STATSDLINE:
        case NORFC1459_RPL_LOCALUSERS:
        case NORFC1459_RPL_GLOBALUSERS:
            {
                int i = 1;
                GString *buf = g_string_new(NULL);

                while (i < count){
                    g_string_append_printf(buf, "%s ", params[i++]);
                }
                srv_hdr_ui_recv_msg(sess->host, META_SERVER,
                        origin, sess->host, buf->str, 0);
                g_string_free(buf, TRUE);
                break;
            }

        case LIBIRC_RFC_ERR_NICKNAMEINUSE:
            {
                int len = strlen(sess->nickname);
                if (len < NICK_LEN - 1){
                    sess->nickname[len] = '_';
                    sess->nickname[len + 1] = 0;
                }
                irc_cmd_nick(irc_session, sess->nickname);
                break;
            }

            /************************ NAMES message ************************/
        case LIBIRC_RFC_RPL_NAMREPLY:
            {
                CHECK_COUNT(4);
                char *nickptr;
                const char *chan = params[2];
                const char *names = params[3];

                nickptr = strtok((char *)names, " ");
                while (nickptr){
                    UserType type;
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
                    srv_hdr_ui_add_user(sess->host, chan, nickptr, type);
                    srv_session_add_user(sess, chan, nickptr);
                    nickptr = strtok(NULL, " ");
                }
                break;
            }

        case LIBIRC_RFC_RPL_TOPIC:
            {
                CHECK_COUNT(3);
                const char *chan = params[1];
                const char *topic = params[2];
                srv_hdr_ui_set_topic(sess->host, chan, topic);
                break;
            }

            /************************ Whois message ************************/
        case LIBIRC_RFC_RPL_WHOISUSER:
            {
                CHECK_COUNT(6);
                snprintf(buf, sizeof(buf), _("%s <%s@%s> %s"), params[1], params[2],
                        params[3], params[5]);
                srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL, 0);
                const char *nick = params[1];
                const char *realname = params[5];
                /* Use Real Name as avatar token :-( */
                plugin_avatar(nick, realname);
                break;
            }
        case LIBIRC_RFC_RPL_WHOISCHANNELS:
            CHECK_COUNT(3);
            snprintf(buf, sizeof(buf), _("%s is member of %s"), params[1], params[2]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL, 0);
            break;
        case LIBIRC_RFC_RPL_WHOISSERVER:
            CHECK_COUNT(4);
            snprintf(buf, sizeof(buf), _("%s is attached to %s at \"%s\""),
                    params[1], params[2], params[3]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL, 0);
            break;
        case LIBIRC_RFC_RPL_WHOISIDLE:
            CHECK_COUNT(5); // TODO
            snprintf(buf, sizeof(buf), _("%s is idle for %s seconds since %s"),
                    params[1], params[2], params[3]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL, 0);
            break;
        case NORFC1459_RPL_WHOWAS_TIME:
            CHECK_COUNT(4);
            snprintf(buf, sizeof(buf), _("%s %s %s"),
                    params[1], params[3], params[2]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL, 0);
            break;
        case NORFC1459_RPL_WHOISHOST:
        case NORFC1459_RPL_WHOISSECURE:
            CHECK_COUNT(3);
            snprintf(buf, sizeof(buf), _("%s %s"), params[1], params[2]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL, 0);
            break;
        case LIBIRC_RFC_RPL_ENDOFWHOIS:
            CHECK_COUNT(3);
            srv_hdr_ui_sys_msg(sess->host, "", params[2], SYS_MSG_NORMAL, 0);
            break;

            /************************ NAMES message ************************/
        case LIBIRC_RFC_RPL_WHOREPLY:
            {
                CHECK_COUNT(8);
                /* params[count - 1] = "<hopcount> <realname>", Skip ' ' */
                const char *nick = params[5];
                const char *realname = strchr(params[count - 1], ' ');
                if (realname) realname++;
                else break;
                /* Use Real Name as avatar token :-( */
                plugin_avatar(nick, realname);
                break;
            }
        case LIBIRC_RFC_RPL_ENDOFWHO:
            break;
        default:
            {
                // Error message
                if (event >= 400 && event < 600){
                    char msg[512];

                    snprintf(msg, sizeof(msg), _("ERROR[%3d]: %s"), event, params[count-1]);
                    srv_hdr_ui_sys_msg(sess->host, "", msg, SYS_MSG_ERROR, 0);
                    return;
                }

                int i;
                LOG_FR("Drop message, session: %s, event: %d, origin: %s, count: %u, params: [",
                        sess->host, event, origin, count);
                for (i = 0; i < count; i++) LOG("'%s', ", params[i]); LOG("]\n");
            }
    }

}
