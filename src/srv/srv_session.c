/**
 * @file srv_session.c
 * @brief Server sessions manager
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __DBG_ON
#define __LOG_ON

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "srv_session.h"
#include "srv_event.h"

#include "log.h"

srv_session_t sessions[MAX_SESSIONS] = { 0 };
irc_callbacks_t cbs;

static int srv_session_get_index(srv_session_t *session){
    int i;
    if (!session){
        WARN_FR("Session is NULL");
        return -1;
    }

    for (i = 0; i < MAX_SESSIONS; i++){
        if (&sessions[i] == session){
            if (session[i].stat == SESS_NOINUSE){
                return -1;
            } else {
                return i;
            }
        }
    }

    return -1;
}

static int srv_session_reconnect(srv_session_t *session){
    int res;

    WARN_FR("Reconnecting, session: %s", session->host);

    // TODO: stat control
    irc_disconnect(session->irc_session);
    res = irc_connect(session->irc_session,
                session->host, session->port,
                session->passwd, session->nickname,
                session->username, session->realname);
    if (res){
        ERR_FR("Failed to connect: %s",
                irc_strerror(irc_errno(session->irc_session)));
        return -1;
    }

    return 0;
}

/**
 * @brief Proecss all sessions' event, This function runs on a separate thread
 * and never return, there is only one thread in a application.
 */
static void _srv_session_proc(){
    int i;
    int maxfd;
    struct timeval tv;
    fd_set in_set, out_set;
    irc_session_t *isess;

    LOG_FR("Networking loop started");

loop:
    maxfd = 0;
    tv.tv_usec = 0;
    tv.tv_sec = 1;

    FD_ZERO(&in_set);
    FD_ZERO(&out_set);

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i].stat == SESS_NOINUSE) continue;
        if ((sessions[i].stat == SESS_CONNECT || sessions[i].stat == SESS_LOGIN)
                && !irc_is_connected(sessions[i].irc_session)){
            if (srv_session_reconnect(&sessions[i]) == -1)
                continue;
        }
        isess = sessions[i].irc_session;

        irc_add_select_descriptors(isess, &in_set, &out_set, &maxfd);
    }

    if (select(maxfd + 1, &in_set, &out_set, 0, &tv) < 0 ){
        ERR_FR("select() error");
        return;
    }

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i].stat == SESS_NOINUSE) continue;
        isess = sessions[i].irc_session;

        if (irc_process_select_descriptors(isess, &in_set, &out_set)){
            // Handle error
        }
    }

    goto loop;
}

void srv_session_init(){
    LOG_FR("...");

    memset(sessions, 0, sizeof(sessions));
    memset (&cbs, 0, sizeof(cbs));

    // Set up the mandatory events
    cbs.event_connect  = srv_event_connect;
    cbs.event_nick = srv_event_nick;
    cbs.event_quit = srv_event_quit;
    cbs.event_join = srv_event_join;
    cbs.event_part = srv_event_part;
    cbs.event_mode = srv_event_mode;
    cbs.event_umode = srv_event_umode;
    cbs.event_topic = srv_event_topic;
    cbs.event_kick = srv_event_kick;
    cbs.event_channel = srv_event_channel;
    cbs.event_privmsg = srv_event_privmsg;
    cbs.event_notice = srv_event_notice;
    cbs.event_channel_notice = srv_event_channel_notice;
    cbs.event_invite = srv_event_invite;
    cbs.event_ctcp_action = srv_event_ctcp_action;
    cbs.event_numeric = srv_event_numeric;
}

void srv_session_proc(){
    // Start the networking loop
    g_thread_new(NULL, (GThreadFunc)_srv_session_proc, NULL);
}

/**
 * @brief Create a srv_session
 *
 * @param host
 * @param port Can be 0, fallback to 6667
 * @param passwd Can be NULL
 * @param nickname
 * @param username Can be NULL
 * @param realname Can be NULL
 *
 * @return NULL or srv_session_t
 */
srv_session_t* srv_session_new(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname){
    int i;
    srv_session_t *sess;

    if (!port) port = 6667;
    if (!passwd) passwd = "";
    if (!username) username = nickname;
    if (!realname) realname = nickname;

    if (srv_session_get_by_host(host) != NULL){
        WARN_FR("Session %s already exist", host);
        return NULL;
    }

    LOG_FR("host: %s, port: %d, nickname: %s", host, port, nickname);

    sess = NULL;
    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i].stat == SESS_NOINUSE){
            sess = &sessions[i];
            break;
        } else {
            continue;
        }
    }
    if (!sess){
        WARN_FR("MAX_SESSIONS count limit");
        return NULL;
    }

    sess->irc_session = irc_create_session(&cbs);

    if (!(sess->irc_session)){
        ERR_FR("Failed to create IRC session");
        return NULL;
    }

    sess->stat = SESS_INUSE;
    sess->port = port;
    strncpy(sess->host, host, HOST_LEN);
    strncpy(sess->passwd, passwd, PASSWD_LEN);
    strncpy(sess->realname, realname, NICK_LEN);
    strncpy(sess->username, username, NICK_LEN);
    strncpy(sess->nickname, nickname, NICK_LEN);

    irc_set_ctx(sess->irc_session, sess);
    irc_option_set(sess->irc_session, LIBIRC_OPTION_STRIPNICKS);

    if (irc_connect(sess->irc_session, host, port, passwd,
                nickname,username, realname)){
        ERR_FR("Failed to connect: %s",
                irc_strerror(irc_errno(sess->irc_session)));
        return NULL;
    }

    return sess;
}

int srv_session_free(srv_session_t *session){
    int i;

    i = srv_session_get_index(session);
    if (i == -1) {
        return -1;
    }

    DBG_FR("Free session %s", session->host);

    irc_destroy_session(session->irc_session);

    sessions[i] = (srv_session_t) { 0 };
    sessions[i].stat = SESS_NOINUSE;

    return 0;
}

srv_session_t* srv_session_get_by_host(const char *host){
    int i;

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i].irc_session == 0) continue;
        if (strcmp(sessions[i].host, host) == 0)
            return &sessions[i];
    }

    return NULL;
}

int srv_session_send(srv_session_t *session,
        const char *target, const char *msg){
    return irc_cmd_msg(session->irc_session, target, msg);
}

int srv_session_me(srv_session_t *session,
        const char *target, const char *msg){
    return irc_cmd_me(session->irc_session, target, msg);
}

int srv_session_join(srv_session_t *session,
        const char *chan, const char *passwd){
    return irc_cmd_join(session->irc_session, chan, passwd);
}

int srv_session_part(srv_session_t *session, const char *chan){
    return irc_cmd_part(session->irc_session, chan);
}

int srv_session_quit(srv_session_t *session, const char *reason){
    return irc_cmd_quit(session->irc_session, reason);
}
