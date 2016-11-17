#ifndef __SRV_SESSION_H
#define __SRV_SESSION_H

#include <glib.h>

#include <libircclient.h>

#define MAX_SESSIONS    10
#define HOST_LEN        128
#define CHAN_LEN        32
#define NICK_LEN        32
#define PASSWD_LEN      32
#define MSG_LEN         512

#define IS_CHAN(x) (x && (x[0] == '#' || x[0] == '&'))

#define SRV_SESSION_FLAG_SSL            0x1
#define SRV_SESSION_FLAG_SSL_NOVERIFY   0x2

typedef int SRVSessionFlag;

typedef enum {
    SRV_SESSION_STAT_NOINUSE = 0,
    SRV_SESSION_STAT_INUSE,
    SRV_SESSION_STAT_CONNECT,
    SRV_SESSION_STAT_CLOSE,
    /*...*/
} SRVSessionStat;

typedef struct{
    char name[CHAN_LEN];
    int joined; // Whether you actually join in this channel
    GList *user_list;
} SRVChannel;

typedef struct {
    // Session status
    SRVSessionStat stat;

    // IRC context
    char prefix[1];   // '#' or ' ', Note: plz use it as server parameter of `irc_connect()`
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN]; // Server password
    char nickname[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];

    GList *chan_list;   // List of channel_t

    irc_session_t *irc_session;
} SRVSession;

void srv_session_init();
void srv_session_proc();
SRVSession* srv_session_new(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname, SRVSessionFlag flag);
int srv_session_is_session(SRVSession *session);
int srv_session_free(SRVSession *session);

SRVSession* srv_session_get_by_host(const char *host);

int srv_session_send(SRVSession *session, const char *target, const char *msg);
int srv_session_me(SRVSession *session, const char *target, const char *msg);
int srv_session_join(SRVSession *session, const char *chan, const char *passwd);
int srv_session_part(SRVSession *session, const char *chan);
int srv_session_quit(SRVSession *session, const char *reason);
void srv_session_quit_all();
int srv_session_nick(SRVSession *session, const char *new_nick);
int srv_session_whois(SRVSession *session, const char *nick);
int srv_session_who(SRVSession *session, const char *nick);
int srv_session_invite(SRVSession *session, const char *nick, const char *chan);
int srv_session_kick(SRVSession *session, const char *nick, const char *chan, const char *reason);
int srv_session_mode(SRVSession *session, const char *target, const char *mode);
int SRVSessionopic(SRVSession *session, const char *chan, const char *topic);

/* Handle session's channel list and user list */
int srv_session_add_chan(SRVSession *session, const char *chan);
int srv_session_rm_chan(SRVSession *session, const char *chan);
int srv_session_add_user(SRVSession *session, const char *chan, const char *nick);
int srv_session_rm_user(SRVSession *session, const char *chan, const char *nick);
int srv_session_user_exist(SRVSession *session, const char *chan, const char *nick);

#endif /* __SRV_SESSION_H */
