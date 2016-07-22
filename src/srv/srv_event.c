/**
 * @file srv_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

// #define __DBG_ON

#include <string.h>

#include "srv_session.h"
#include "srv_event.h"
#include "srv_hdr.h"

#include "i18n.h"
#include "log.h"

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

void srv_event_connect(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    irc_cmd_join(irc_session, "#srain", 0);
    irc_cmd_join(irc_session, "#srain2", 0);
    sess->stat = SESS_CONNECT;

    // TODO: ??
}

void srv_event_nick(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    const char *new_nick;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(1);
    new_nick = params[0];

    srv_hdr_ui_user_list_rename(sess->host, NULL, origin, new_nick, 0);
}

void srv_event_quit(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    const char *reason;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(1);
    reason = params[0];

    srv_hdr_ui_user_list_rm(sess->host, NULL, origin, reason);
}

void srv_event_join(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(1);
    const char *chan = params[0];

    // TODO: more prefix
    if (origin[0] == '@')
        srv_hdr_ui_user_list_add(sess->host, chan, origin, USER_FULL_OP, 1);
    else
        srv_hdr_ui_user_list_add(sess->host, chan, origin, USER_CHIGUA, 1);
}

void srv_event_part(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;
    CHECK_COUNT(2);
    const char *chan = params[0];
    const char *reason = params[1];

    srv_hdr_ui_user_list_rm(sess->host, origin, params[0], reason);
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

    // TODO: remove from user list
    snprintf(msg, sizeof(msg), _("%s are kicked from %s by %s: %s"),
            kick_nick, chan, origin, reason);
    srv_hdr_ui_sys_msg(sess->host, chan, msg, SYS_MSG_ERROR);
}

void srv_event_channel(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    const char *chan;
    const char *msg;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    chan = params[0];
    msg = params[1];

    // TODO: id
    srv_hdr_ui_recv_msg(sess->host, chan, origin, "", msg);
}

void srv_event_privmsg(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    const char *dest;
    const char *msg;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    PRINT_EVENT_PARAM;

    CHECK_COUNT(2);
    dest = params[0];
    msg = params[1];

    // TODO: id
    srv_hdr_ui_recv_msg(sess->host, dest, origin, "", msg);
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
    const char *chan = params[0];

    snprintf(msg, sizeof(msg), _("%s invites you into %s"), origin, chan);
    srv_hdr_ui_sys_msg(sess->host, NULL, msg, SYS_MSG_NORMAL);
}

void srv_event_numeric (irc_session_t *irc_session, unsigned int event,
        const char *origin, const char **params, unsigned int count){
    int i;
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);

    DBG_F("session: %s, event: %d, origin: %s, count: %u, params: [",
            sess->host, event, origin, count);
    for (i = 0; i < count; i++) DBG("'%s', ", params[i]); DBG("]\n");

    switch (event){
        case LIBIRC_RFC_RPL_WELCOME:
            DBG_FR("You are logined as %s", params[0]);
            sess->stat = SESS_LOGIN;
            break;
        case LIBIRC_RFC_ERR_NICKNAMEINUSE:
            ;   // :-|
            int len = strlen(sess->nickname);
            if (len < NICK_LEN - 1){
                sess->nickname[len] = '_';
                sess->nickname[len + 1] = 0;
            }
            irc_cmd_nick(irc_session, sess->nickname);
            break;
        case LIBIRC_RFC_RPL_NAMREPLY:
            // TODO
            break;
        case LIBIRC_RFC_RPL_TOPIC:
            ;
            CHECK_COUNT(3)
            const char *chan = params[1];
            const char *topic = params[2];

            srv_hdr_ui_set_topic(sess->host, chan, topic);
    }
}
