#ifndef __SRV_SESSION_H
#define __SRV_SESSION_H

#include <glib.h>

#include "libircclient.h"
#include "libirc_rfcnumeric.h"

#define MAX_SESSIONS    10
#define HOST_LEN        128
#define CHAN_LEN        32
#define NICK_LEN        32
#define PASSWD_LEN      32
#define MSG_LEN         512

#define IS_CHAN(x) (x && (x[0] == '#' || x[0] == '&'))

typedef enum {
    SSL_OFF = 0,
    SSL_NO_VERIFY,
    SSL_ON,
    /*...*/
} ssl_opt_t;

typedef enum {
    SESS_NOINUSE = 0,
    SESS_INUSE,
    SESS_CONNECT,
    SESS_CLOSE,
    /*...*/
} session_stat_t;

typedef struct {
    // Session status
    session_stat_t stat;

    // IRC context
    char prefix[1];   // '#' or ' ', Note: plz use it as server parameter of `irc_connect()`
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN]; // Server password
    char nickname[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];

    GList *chans;

    irc_session_t *irc_session;
} srv_session_t;

void srv_session_init();
void srv_session_proc();
srv_session_t* srv_session_new(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname, ssl_opt_t ssl);
int srv_session_is_session(srv_session_t *session);
int srv_session_free(srv_session_t *session);

srv_session_t* srv_session_get_by_host(const char *host);

int srv_session_send(srv_session_t *session, const char *target, const char *msg);
int srv_session_me(srv_session_t *session, const char *target, const char *msg);
int srv_session_join(srv_session_t *session, const char *chan, const char *passwd);
int srv_session_part(srv_session_t *session, const char *chan);
int srv_session_quit(srv_session_t *session, const char *reason);
void srv_session_quit_all();
int srv_session_nick(srv_session_t *session, const char *new_nick);
int srv_session_whois(srv_session_t *session, const char *nick);
int srv_session_invite(srv_session_t *session, const char *nick, const char *chan);
int srv_session_kick(srv_session_t *session, const char *nick, const char *chan, const char *reason);
int srv_session_mode(srv_session_t *session, const char *target, const char *mode);
int srv_session_topic(srv_session_t *session, const char *chan, const char *topic);

#endif /* __SRV_SESSION_H */
