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

#include "srv_session.h"
#include "srv_event.h"
#include "srv_hdr.h"

#include "i18n.h"
#include "log.h"
#include "filter.h"

#define PRINT_EVENT_PARAM \
    do { \
        DBG_F("session: %s, event: %s, origin: %s, count %u, params: [", \
                sess->host, event, origin, count); \
        int i; \
        for (i = 0; i < count; i++) DBG("'%s', ", params[i]); \
        DBG("]\n"); \
    } while (0)

#define CHECK_COUNT(x) \
    if (count != x) { \
        ERR_FR("sessions: %s, count: %u, except %u", sess->host, count, x); \
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
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    sess->stat = SESS_CONNECT;

    LOG_FR("Connected to %s", sess->host);

    srv_hdr_ui_add_chan(sess->host, SRV_SESSION_SERVER);
}

void srv_event_nick(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    const char *new_nick;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(1);
    new_nick = params[0];

    snprintf(msg, sizeof(msg), _("%s is now known as %s"), origin, new_nick);
    srv_hdr_ui_user_list_rename(sess->host, origin, new_nick, 0, msg);
}

void srv_event_quit(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    const char *reason;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(1);
    reason = params[0];

    snprintf(msg, sizeof(msg), _("%s has quit: %s"), origin, reason);
    srv_hdr_ui_user_list_rm_all(sess->host, origin, msg);

    /* You quit */
    if (strncasecmp(origin, sess->nickname, NICK_LEN) == 0){
        /* Remove all chans belong to this session */
        srv_hdr_ui_rm_chan(sess->host, "");
        srv_session_free(sess);
        sess->stat = SESS_NOINUSE;
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
        srv_hdr_ui_add_chan(sess->host, chan);
    }

    // TODO: prefix?
    srv_hdr_ui_user_list_add(sess->host, chan, origin, USER_CHIGUA);

    snprintf(msg, sizeof(msg), _("%s has joined %s"), origin, chan);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL);
}

void srv_event_part(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);
    const char *chan = params[0];
    const char *reason = params[1];

    snprintf(msg, sizeof(msg), _("%s has left %s: %s"), origin, chan, reason);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL);
    srv_hdr_ui_user_list_rm(sess->host, chan, origin);

    /* YOU has left a channel */
    if (strncasecmp(sess->nickname, origin, NICK_LEN) == 0){
        srv_hdr_ui_rm_chan(sess->host, chan);
    }
}

void srv_event_mode(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(3);
    const char *chan = params[0];
    const char *mode = params[1];
    const char *mode_arg = params[2];

    snprintf(msg, sizeof(msg), _("mode %s %s %s by %s"),
            chan, mode, mode_arg, origin);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL);
    // TODO
}

void srv_event_umode(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);
    const char *chan = params[0];
    const char *mode = params[1];

    snprintf(msg, sizeof(msg), _("mode %s %s by %s"),
            chan, mode, origin);

    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_NORMAL);
}

void srv_event_topic(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    const char *chan;
    const char *topic;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    chan = params[0];
    topic = params[1];

    srv_hdr_ui_set_topic(sess->host, chan, topic);
}

void srv_event_kick(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(3);
    const char *chan = params[0];
    const char *kick_nick = params[1];
    const char *reason = params[2];

    snprintf(msg, sizeof(msg), _("%s are kicked from %s by %s: %s"),
            kick_nick, chan, origin, reason);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_ERROR);

    srv_hdr_ui_user_list_rm(sess->host, chan, kick_nick);
}

void srv_event_channel(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    const char *chan;
    const char *msg;
    char vmsg[MSG_LEN];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    chan = params[0];
    msg = params[1];

    strncpy(vmsg, msg, MSG_LEN);
    strip(vmsg);

    char nick[NICK_LEN] = { 0 };
    filter_relaybot_trans(origin, nick, vmsg);

    /* A message sent by relay bot */
    if (strlen(nick) > 0){
        if (!filter_is_ignore(nick))
            srv_hdr_ui_recv_msg(sess->host, chan, nick, origin, vmsg);
    } else {
        if (!filter_is_ignore(origin))
            srv_hdr_ui_recv_msg(sess->host, chan, origin, "", vmsg);
    }
}

void srv_event_privmsg(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char *msg;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    msg = strdup(params[1]);

    strip(msg);

    if (!filter_is_ignore(origin))
        // TODO: fall back to SRV_SESSION_SERVER
        srv_hdr_ui_recv_msg(sess->host, origin, origin, "", msg);

    free(msg);
}

void srv_event_notice(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);
    const char *msg = params[0];

    srv_hdr_ui_recv_msg(sess->host, origin, origin, "", msg);
}

void srv_event_channel_notice(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);
    const char *chan = params[0];
    const char *msg = params[1];

    srv_hdr_ui_recv_msg(sess->host, chan, origin, "", msg);
}

void srv_event_invite(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    const char *chan = params[1];

    snprintf(msg, sizeof(msg), _("%s invites you into %s"), origin, chan);
    srv_hdr_ui_sys_msg(sess->host, "", msg, SYS_MSG_NORMAL);
}

void srv_event_ctcp_action(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    char msg[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    const char *chan = params[0];
    const char *msg1 = params[1];

    snprintf(msg, sizeof(msg), _("*** %s %s ***"), origin, msg1);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_ACTION);
}

void srv_event_numeric (irc_session_t *irc_session, unsigned int event,
        const char *origin, const char **params, unsigned int count){
    int i;
    char buf[512];
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    DBG_F("session: %s, event: %d, origin: %s, count: %u, params: [",
            sess->host, event, origin, count);
    for (i = 0; i < count; i++) DBG("'%s', ", params[i]); DBG("]\n");

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
        case 250:
        case 265:
        case 266:
            {
                int i = 1;
                GString *buf = g_string_new(NULL);

                while (i < count){
                    g_string_append_printf(buf, "%s ", params[i++]);
                }
                srv_hdr_ui_recv_msg(sess->host, SRV_SESSION_SERVER,
                        origin, sess->host, buf->str);
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

            /************************ Whois message ************************/
        case LIBIRC_RFC_RPL_NAMREPLY:
            {
                CHECK_COUNT(4);
                char *nickptr;
                const char *chan = params[2];
                const char *names = params[3];

                nickptr = strtok((char *)names, " ");
                while (nickptr){
                    // TODO: more prefix
                    srv_hdr_ui_user_list_add(sess->host, chan, nickptr, USER_CHIGUA);
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
            CHECK_COUNT(5);
            snprintf(buf, sizeof(buf), _("%s <%s@%s> %s"), params[1], params[2],
                    params[3], params[4]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL);
            break;
        case LIBIRC_RFC_RPL_WHOISCHANNELS:
            CHECK_COUNT(3);
            snprintf(buf, sizeof(buf), _("%s is member of %s"), params[1], params[2]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL);
            break;
        case LIBIRC_RFC_RPL_WHOISSERVER:
            CHECK_COUNT(4);
            snprintf(buf, sizeof(buf), _("%s is attached to %s at \"%s\""),
                    params[1], params[2], params[3]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL);
            break;
        case LIBIRC_RFC_RPL_WHOISIDLE:
            CHECK_COUNT(5); // TODO
            snprintf(buf, sizeof(buf), _("%s is idle for %s seconds since %s"),
                    params[1], params[2], params[3]);
            srv_hdr_ui_sys_msg(sess->host, "", buf, SYS_MSG_NORMAL);
            break;
            // TODO 378 330
        case LIBIRC_RFC_RPL_ENDOFWHOIS:
            CHECK_COUNT(2);
            srv_hdr_ui_sys_msg(sess->host, "", params[1], SYS_MSG_NORMAL);
            break;
    }

    // Error message
    if (event >= 400 && event < 600){
        char msg[512];

        snprintf(msg, sizeof(msg), _("ERROR[%3d]: %s"), event, params[count-1]);
        srv_hdr_ui_sys_msg(sess->host, "", msg, SYS_MSG_ERROR);
        return;
    }
}
