#ifndef __SERVER_H
#define __SERVER_H

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

typedef struct _Message Message;
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
    // SuiUser *ui;
};

struct _Message {
    Chat *chat;
    User *user;     // Originator of this message, often refers to an existing user
    char *dname;    // Decorated name, maybe contains xml tags
    char *role;     // The role of the message originator

    char *content;
    char *dcontent; // Decorated message content
    time_t time;
    bool mentioned;

    GSList *urls;   // URLs contains in message, like "http://xxx", "irc://xxx"
    // SuiMessage *ui;
};

/* Represent a channel or dialog */
struct _Chat {
    char name[CHAN_LEN];
    bool joined;
    User *me;

    GSList *user_list;
    GList *msg_list;
    /*
    GSList *ignore_list;
    GSList *brigebot_list;
    */

    Server *srv;
    SuiSession *ui;
};

struct _Server {
    volatile ServerStatus stat;
    // ServerInfo info;

    /* Server profile */
    char name[NAME_LEN];
    char host[HOST_LEN];
    int port;
    char passwd[PASSWD_LEN];
    bool ssl;
    char *encoding;

    time_t last_ping;
    User *user;     // Used to store your nick, username, realname
    Chat *chat;     // Hold all messages that do not belong to any other Chat

    GSList *chat_list;

    GSList *ignore_list;
    GSList *brigebot_list;

    SircSession *irc;
};

// TODO: member of Server
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
int server_add_chat(Server *srv, const char *name);
int server_rm_chat(Server *srv, const char *name);
Chat* server_get_chat(Server *srv, const char *name);
Chat* server_get_chat_fallback(Server *srv, const char *name);

Chat *chat_new(Server *srv, const char *name);
void chat_free(Chat *chat);
int chat_add_user(Chat *chat, const char *nick, UserType type);
int chat_rm_user(Chat *chat, const char *nick);
User* chat_get_user(Chat *chat, const char *nick);

User *user_new(Chat *chat, const char *nick, const char *username, const char *realname, UserType type);
void user_free(User *user);
void user_rename(User *user, const char *new_nick);
void user_set_type(User *user, UserType type);

Message* message_new(Chat *chat, User *user, const char *content);
void message_free(Message *msg);

#endif /* __SERVER_H */
