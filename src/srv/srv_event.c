/**
 * @file srv_event.c
 * @brief Server event callbacks
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __DBG_ON

#include "srv_session.h"
#include "srv_event.h"

#include "log.h"

void srv_event_connect(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);

    sess->stat = SESS_CONNECT;
}

void srv_event_nick(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_quit(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_join(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_part(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_mode(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_umode(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_topic(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_kick(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_channel(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_privmsg(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_notice(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_channel_notice(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_invite(irc_session_t *irc_session, const char *event,
 const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %s", sess->host, event);
}

void srv_event_numeric (irc_session_t *irc_session, unsigned int event,
        const char *origin, const char **params, unsigned int count){
    srv_session_t *sess;

    sess = irc_get_ctx(irc_session);
    DBG_FR("session: %s, event: %d", sess->host, event);

    switch (event){
        case LIBIRC_RFC_RPL_WELCOME:
            DBG_FR("LIBIRC_RFC_RPL_WELCOME %d", count);
            DBG_FR("You are logined as %s", params[0]);
            sess->stat = SESS_LOGIN;
            break;
        case LIBIRC_RFC_RPL_NAMREPLY:
            DBG_FR("LIBIRC_RFC_RPL_NAMREPLY %d", event);
        case LIBIRC_RFC_ERR_NICKNAMEINUSE:
            WARN_FR("LIBIRC_RFC_ERR_NICKNAMEINUSE %d", event);
            irc_cmd_nick(irc_session, "la_");
    }
}
