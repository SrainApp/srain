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

#include "libircclient.h"
#include "libirc_rfcnumeric.h"

#include "log.h"

#define MAX_SESSIONS    10
#define HOST_LEN        256
#define CHAN_LEN        200
#define NICK_LEN        128

// Session context
typedef struct {
    char host[HOST_LEN];
    char nick[NICK_LEN];
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

static void event_connect(irc_session_t *session, const char *event,
        const char* origin, const char ** params, unsigned int count){
    LOG_FR("Connected: %s %s", event, origin);
}

static void event_numeric (irc_session_t * session, unsigned int event,
        const char* origin, const char ** params, unsigned int count){
    LOG_FR("Numeric: %d %s", event, origin);
}

void irc_init(){
    memset(sessions, 0, sizeof(sessions));
    memset(ctxs, 0, sizeof(ctxs));
    memset (&cbs, 0, sizeof(cbs));

    // Set up the mandatory events
    cbs.event_connect = event_connect;
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
            // ctx = irc_get_ctx(sessions[i]);
            // irc_connect(sessions[i], ctx->host, ctx->nick, )
            // TODO: reconnect
            ERR_FR("No connected");
            continue;
        }
        session = sessions[i];
        LOG_FR("Add session %p", session);

        irc_add_select_descriptors(session, &in_set, &out_set, &maxfd);
    }

    if (select(maxfd + 1, &in_set, &out_set, 0, &tv) < 0 ){
        ERR_FR("select() error");
        return -1;
    }

    for (i = 0; i < MAX_SESSIONS; i++){
        if (sessions[i] == 0) continue;
        session = sessions[i];

        LOG_FR("Process session %p", session);
        if (irc_process_select_descriptors(session, &in_set, &out_set)){
        }
    }

    return 0;
}

int irc_session_new(const char *host, int port, const char *nickname,
        const char *username, const char *realname){
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

    if (irc_connect(session, host, port, 0, nickname, username, realname)){
        ERR_FR("Failed to connect: %s", irc_strerror(irc_errno(session)));
        return -1;
    }

    return 0;
}
