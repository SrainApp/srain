#ifndef __SRV_H
#define __SRV_H

#include <stdbool.h>
#include <glib.h>

#include "sirc/sirc.h"
#include "sui/sui.h"

/* Structure members length */
#define NAME_LEN        64
#define PASSWD_LEN      64
#define HOST_LEN        128
#define NICK_LEN        128
#define SERVER_LEN      128
#define USER_LEN        128
#define CHAN_LEN        200
#define MSG_LEN         512

typedef struct _User User;
typedef struct _Chat Chat;
typedef struct _Server Server;
typedef struct _ServerInfo ServerInfo;

typedef enum {
    SERVER_UNCONNECTED,
    SERVER_CONNECTING,
    SERVER_CONNECTED,
    SERVER_DISCONNECTED,
} ServerStatus;

struct _User {
    char nick[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];

    bool me;
    UserType type;

    Chat *chat;
};

/* Represent a channel or dialog */
struct _Chat {
    char name[CHAN_LEN];
    char passwd[PASSWD_LEN];
    bool joined;
    User *me;
    GList *user_list;

    Server *srv;
    SuiSession *ui;
};

struct _Server {
    /* Server profile */
    char name[NAME_LEN];
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN];
    bool ssl;
    char *encoding;

    time_t last_ping;
    GList *chat_list;
    User user;

    volatile ServerStatus stat;
    SuiSession *ui;
    SircSession *irc;
};

struct _ServerInfo {
    char name[NAME_LEN];
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN];
    bool ssl;
    char *encoding;
};

void server_init();
void server_finalize();

Server* server_new(const char *name, const char *host, int port,
        const char *passwd, bool ssl, const char *enconding,
        const char *nick, const char *username, const char *realname);
void server_free(Server *srv);
int server_connect(Server *srv);
void server_disconnect(Server *srv);

int server_add_chat(Server *srv, const char *name, const char *passwd);
int server_rm_chat(Server *srv, const char *name);
Chat* server_get_chat(Server *srv, const char *name);

int chat_add_user(Chat *chat, const char *nick, UserType type);
int chat_rm_user(Chat *chat, const char *nick);
User* chat_get_user(Chat *chat, const char *nick);

int user_rename(User *user, const char *new_nick);
int user_set_type(User *user, UserType type);

#endif /* __SRV_H */
