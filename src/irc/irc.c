/**
 * @file irc.c
 * @brief IRC sessions manager
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-17
 */

#define __LOG_ON

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "libircclient.h"
#include "libirc_rfcnumeric.h"

#include "log.h"

#define MAX_SESSIONS    10
#define HOST_LEN        INET6_ADDRSTRLEN
#define CHAN_LEN        64
#define NICK_LEN        64
#define PASSWORD_LEN    64


// Session context
typedef struct {
    char host[INET6_ADDRSTRLEN];
    int port;
    char password[PASSWORD_LEN]; // Server password
    char nick[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];
} irc_ctx_t;

irc_session_t *sessions[MAX_SESSIONS] = { NULL };
irc_ctx_t ctxs[MAX_SESSIONS] = { 0 };
irc_callbacks_t cbs;

static irc_session_t* get_session_by_host(const char *host){
    int i;
    irc_ctx_t *ctx;

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i] == 0) continue;
        ctx = irc_get_ctx(sessions[i]);
        if (strcmp(ctx->host, host) == 0) return sessions[i];
    }

    return NULL;
}

static const char* get_session_host(irc_session_t *session){
    irc_ctx_t *ctx;

    ctx = irc_get_ctx(session);

    return ctx->host;
}

/******************** Callbacks ********************/
static void event_connect(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
    irc_cmd_join(session, "#srain", 0);
}

static void event_nick(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_quit(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_join(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_part(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_mode(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_umode(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_topic(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_kick(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_channel(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_privmsg(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_notice(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_channel_notice(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_invite(irc_session_t *session, const char *event,
 const char *origin, const char **params, unsigned int count){
    LOG_FR("session: %s, event: %s", get_session_host(session), event);
}

static void event_numeric (irc_session_t *session, unsigned int event,
        const char *origin, const char **params, unsigned int count){
    int i;

    LOG_FR("session: %s, event: %d", get_session_host(session), event);
    switch (event){
        case LIBIRC_RFC_RPL_WELCOME:
            LOG_FR("LIBIRC_RFC_RPL_WELCOME %d", count);
            LOG_FR("You are logined as %s", params[0]);
            break;
        case LIBIRC_RFC_RPL_NAMREPLY:
            LOG_FR("LIBIRC_RFC_RPL_NAMREPLY %d", count);
            for (i = 0; i < count; i++)
                LOG_FR("params %s", params[i]);
            break;
        case LIBIRC_RFC_ERR_NICKNAMEINUSE:
            LOG_FR("LIBIRC_RFC_ERR_NICKNAMEINUSE %d", count);
            irc_cmd_nick(session, "la_");
    }

}
/***************************************************/

static int irc_session_reconnect(irc_session_t *session){
    int retry;
    irc_ctx_t *ctx;

    retry = 10;
    ctx = irc_get_ctx(session);

    while (retry--){
        ERR_FR("Reconnecting, session: %s, left times: %d",
                get_session_host(session), retry);
        irc_connect(session, ctx->host, ctx->port, ctx->password,
                ctx->nick, ctx->username, ctx->realname);
        if (irc_is_connected(session))
            return 0;
        sleep(1);
    }

    return -1;
}

void irc_init(){
    memset(sessions, 0, sizeof(sessions));
    memset(ctxs, 0, sizeof(ctxs));
    memset (&cbs, 0, sizeof(cbs));

    // Set up the mandatory events
    cbs.event_connect  = event_connect;
    cbs.event_nick = event_nick;
    cbs.event_quit = event_quit;
    cbs.event_join = event_join;
    cbs.event_part = event_part;
    cbs.event_mode = event_mode;
    cbs.event_umode = event_umode;
    cbs.event_topic = event_topic;
    cbs.event_kick = event_kick;
    cbs.event_channel = event_channel;
    cbs.event_privmsg = event_privmsg;
    cbs.event_notice = event_notice;
    cbs.event_channel_notice = event_channel_notice;
    cbs.event_invite = event_invite;
    cbs.event_numeric = event_numeric;
}

int irc_session_process(){
    int i;
    int maxfd;
    struct timeval tv;
    fd_set in_set, out_set;
    irc_ctx_t *ctx;
    irc_session_t *session;

    maxfd = 0;
    tv.tv_usec = 250000;
    tv.tv_sec = 0;

    FD_ZERO(&in_set);
    FD_ZERO(&out_set);

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i] == 0) continue;
        if (!irc_is_connected(sessions[i])){
            if (irc_session_reconnect(sessions[i]) == -1)
                continue;
        }
        session = sessions[i];

        irc_add_select_descriptors(session, &in_set, &out_set, &maxfd);
    }

    if (select(maxfd + 1, &in_set, &out_set, 0, &tv) < 0 ){
        ERR_FR("select() error");
        return -1;
    }

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i] == 0) continue;
        session = sessions[i];

        if (irc_process_select_descriptors(session, &in_set, &out_set)){
        }
    }

    return 0;
}

int irc_session_new(const char *host, int port, const char *password,
        const char *nickname, const char *username, const char *realname){
    int i;
    irc_session_t *session;
    irc_ctx_t *ctx;

    LOG_FR("host: %s, port: %d, nickname: %s", host, port, nickname);

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i] == 0){
            break;
        } else {
            continue;
        }
    }
    if (i == MAX_SESSIONS && sessions[MAX_SESSIONS-1] != 0){
        ERR_FR("MAX_SESSIONS count limit");
        return -1;
    }

    sessions[i] = irc_create_session(&cbs);

    if (!sessions[i]){
        ERR_FR("Failed to create IRC session");
        return -1;
    }

    session = sessions[i];
    ctx = &ctxs[i];

    strncpy(ctx->host, host, HOST_LEN);
    strncpy(ctx->nick, nickname, NICK_LEN);

    irc_set_ctx(session, ctx);
    irc_option_set(session, LIBIRC_OPTION_STRIPNICKS);

    if (irc_connect(session, host, port, password,
                nickname,username, realname)){
        ERR_FR("Failed to connect: %s", irc_strerror(irc_errno(session)));
        return -1;
    }

    return 0;
}
