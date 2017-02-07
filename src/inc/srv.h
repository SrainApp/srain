#ifndef __SRV_H
#define __SRV_H

#include <stdbool.h>
#include <glib.h>

#include "sirc.h"

/* Structure members length */
#define NAME_LEN        64
#define PASSWD_LEN      64
#define HOST_LEN        128
#define NICK_LEN        128
#define SERVER_LEN      128
#define USER_LEN        128
#define CHAN_LEN        200
#define MSG_LEN         512

#define BUF_LEN         512

/* Ref: http://www.geekshed.net/2009/10/nick-prefixes-explained/ */
typedef enum {
    USER_CHIGUA,    // No prefix
    USER_OWNER,     // ~ mode +q
    USER_ADMIN,     // & mode +a
    USER_FULL_OP,   // @ mode +o
    USER_HALF_OP,   // % mode +h
    USER_VOICED,    // + mode +v
    /* ... */
    USER_TYPE_MAX
} UserType;

typedef struct {
    char nick[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];

    bool me;
    UserType type;
} User;

typedef enum {
    SERVER_UNCONNECTED,
    SERVER_CONNECTING,
    SERVER_CONNECTED,
    SERVER_DISCONNECTED,
} ServerStatus;

typedef struct {
    char name[NAME_LEN];
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN];
    bool ssl;
    char *encoding;
} ServerInfo;

typedef struct {
    /* Server profile */
    char name[NAME_LEN];
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN];
    bool ssl;
    char *encoding;

    time_t last_ping;
    GList *chan_list;
    User user;

    volatile ServerStatus stat;
    void *ui;
    SircSession *irc;
} Server;

typedef struct {
    char name[CHAN_LEN];
    char passwd[PASSWD_LEN];
    User *me;
    GList *user_list;

    void *ui;
    Server *srv;
} Channel;

typedef struct {
    User sender;
    char content[MSG_LEN];

    // Channel *sess;
} Message;

void srv_init();
void srv_finalize();

Server* server_new(const char *name, const char *host, int port,
        const char *passwd, bool ssl, const char *enconding,
        const char *nick, const char *username, const char *realname);
void server_free(Server *srv);
int server_connect(Server *srv);
void server_disconnect(Server *srv);

int server_add_chan(Server *srv, const char *name, const char *passwd);
int server_rm_chan(Server *srv, const char *name);
Channel* server_get_chan(Server *srv, const char *name);

int server_add_user(Server *srv, const char *chan_name, const char *nick);
void server_rm_user(Server *srv, const char *chan_name, const char *nick);
User* server_get_user(Server *srv, const char *chan_name, const char *nick);

#endif /* __SRV_H */
