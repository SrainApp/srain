#ifndef __SRV_SESSION_H
#define __SRV_SESSION_H

#include <netdb.h>

#include "libircclient.h"
#include "libirc_rfcnumeric.h"

#define MAX_SESSIONS    10
#define HOST_LEN        128
#define CHAN_LEN        32
#define NICK_LEN        32
#define PASSWD_LEN      32
#define MSG_LEN         512

#define SRV_SESSION_SERVER "Server "

typedef enum {
    SESS_NOINUSE = 0,
    SESS_INUSE = 1,
    SESS_CONNECT = 2,
    SESS_LOGIN = 3,
    /*...*/
} session_stat_t;

typedef struct {
    // Session status
    session_stat_t stat;

    // IRC context
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN]; // Server password
    char nickname[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];

    irc_session_t *irc_session;
} srv_session_t;

void srv_session_init();
void srv_session_proc();
srv_session_t* srv_session_new(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname);
int srv_session_free(srv_session_t *session);

srv_session_t* srv_session_get_by_host(const char *host);

int srv_session_send(srv_session_t *session, const char *target, const char *msg);
int srv_session_me(srv_session_t *session, const char *target, const char *msg);
int srv_session_join(srv_session_t *session, const char *chan, const char *passwd);
int srv_session_part(srv_session_t *session, const char *chan);
int srv_session_quit(srv_session_t *session, const char *reason);

#endif /* __SRV_SESSION_H */
