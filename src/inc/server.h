/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SERVER_H
#define __SERVER_H

#include <glib.h>

#include "sirc/sirc.h"
#include "sui/sui.h"
#include "server_cap.h"
#include "ret.h"

/* In millseconds */
#define SERVER_PING_INTERVAL    (30 * 1000)
#define SERVER_PING_TIMEOUT     (SERVER_PING_INTERVAL * 2)
#define SERVER_RECONN_INTERVAL  (5 * 1000)
#define SERVER_RECONN_STEP      SERVER_RECONN_INTERVAL

/* In seconds */
#define MESSAGE_MERGE_INTERVAL  60

/* Structure members length */
#define NAME_LEN        64
#define PASSWD_LEN      64
#define HOST_LEN        128
#define NICK_LEN        128
#define SERVER_LEN      128
#define USER_LEN        128
#define CHAN_LEN        200

typedef enum   _MessageType MessageType;
typedef struct _Message Message;
// typedef struct _UserType UserType;
typedef struct _User User;
typedef struct _Chat Chat;
typedef struct _ChatPrefs ChatPrefs;
typedef enum   _ServerState ServerState;
typedef enum   _ServerAction ServerAction;
typedef struct _Server Server;
typedef enum   _LoginMethod LoginMethod;
typedef struct _ServerPrefs ServerPrefs;
typedef struct _EnabledCap EnabledCap;
typedef struct _ServerCap ServerCap;


/*enum _UserType {
    USER_CHIGUA,    // No prefix
    USER_OWNER,     // ~ mode +q
    USER_ADMIN,     // & mode +a
    USER_FULL_OP,   // @ mode +o
    USER_HALF_OP,   // % mode +h
    USER_VOICED,    // + mode +v
    USER_TYPE_MAX
}; */

struct _User {
    char nick[NICK_LEN];
    char username[NICK_LEN];
    char realname[NICK_LEN];

    bool me;
    int refcount;
    UserType type;

    Chat *chat;
    // SuiUser *ui;
};

enum _MessageType {
    MESSAGE_UNKNOWN,
    MESSAGE_RECV,
    MESSAGE_SENT,
    MESSAGE_ACTION,
    MESSAGE_NOTICE,
    MESSAGE_MISC,
    MESSAGE_ERROR,
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
    MessageType type;

    GSList *urls;   // URLs contains in message, like "http://xxx", "irc://xxx"
    SuiMessage *ui;
};

/* Represent a channel or dialog or a server session */
struct _Chat {
    char name[CHAN_LEN];
    bool joined;
    User *user;         // Yourself

    GSList *user_list;
    GList *msg_list;
    Message *last_msg;

    /* Used by Filters & Decorators */
    GSList *ignore_nick_list;
    GSList *ignore_regex_list;
    GSList *relaybot_list;

    Server *srv;
    SuiSession *ui;
    ChatPrefs *prefs;
};

struct _ChatPrefs {
    SuiPrefs *ui;
};

enum _ServerState {
    SERVER_STATE_DISCONNECTED,
    SERVER_STATE_CONNECTING,
    SERVER_STATE_CONNECTED,
    SERVER_STATE_DISCONNECTING,
    SERVER_STATE_QUITING,
    SERVER_STATE_RECONNECTING,
};

enum _ServerAction {
    SERVER_ACTION_CONNECT,
    SERVER_ACTION_CONNECT_FAIL,
    SERVER_ACTION_CONNECT_FINISH,
    SERVER_ACTION_DISCONNECT,
    SERVER_ACTION_QUIT,
    SERVER_ACTION_RECONNECT,
    SERVER_ACTION_DISCONNECT_FINISH,
};

struct _Server {
    /* Status */
    ServerState state;
    ServerAction last_action;
    bool negotiated;    // Client capability negotiation has finished
    bool registered;    // User has a nickname
    bool loggedin;      // User has identified as a certain account

    /* Keep alive */
    unsigned long last_pong;        // Last pong time, in ms
    unsigned long delay;            // Delay in ms
    unsigned long reconn_interval;  // Interval of next reconnect, in ms
    int ping_timer;
    int reconn_timer;

    ServerCap *cap;     // Server capabilities
    ServerPrefs *prefs; // All required static informations
    User *user;         // Used to store your nick, username, realname
    Chat *chat;         // Hold all messages that do not belong to any other Chat

    Chat *cur_chat;
    GSList *chat_list;

    SircSession *irc;
};

enum _LoginMethod {
    LOGIN_NONE,
    LOGIN_PASS,
    LOGIN_NICKSERV,
    LOGIN_MSG_NICKSERV,
    LOGIN_SASL_PLAIN,
    LOGIN_UNKNOWN,
};

struct _ServerPrefs {
    /* For specificed server */
    bool predefined;  /* A ServerPrefs is predefined when it is loaded from
                         configuration file, a predefined ServerPrefs will
                         appeared in predefined server list and *CAN NOT* be
                         freed by ``server_prefs_free()``. */
    char *name;
    char *host;
    int port;
    char *passwd;

    /* User */
    char *nickname;
    char *username;
    char *realname;
    char *user_passwd;
    LoginMethod login_method;

    /* Default message */
    char *part_message;
    char *kick_message;
    char *away_message;
    char *quit_message;

    SircPrefs *irc;

    // ...
    Server *srv;
};

struct _EnabledCap {
    // Version 3.1
    bool identify_msg;
    bool mulit_prefix;
    bool away_notify;
    bool account_notify;
    bool extended_join;
    bool sasl;

    // Version 3.2
    bool server_time;
    bool userhost_in_names;
    bool cap_notify;
    bool chghost;

    // Vendor-Specific
    bool znc_server_time_iso;
    bool znc_server_time;
};

struct _ServerCap {
    /* Capabilities */
    EnabledCap client_enabled;
    EnabledCap server_enabled;

    Server *srv;
};

void server_init_and_run(int argc, char *argv[]);
void server_finalize();

Server* server_new_from_prefs(ServerPrefs *prefs);
Server *server_get_by_name(const char *name);
void server_free(Server *srv);
bool server_is_valid(Server *srv);
SrnRet server_connect(Server *srv);
SrnRet server_disconnect(Server *srv);
SrnRet server_state_transfrom(Server *srv, ServerAction act);
bool server_is_registered(Server *srv);
void server_wait_until_registered(Server *srv);
int server_add_chat(Server *srv, const char *name);
int server_rm_chat(Server *srv, const char *name);
Chat* server_get_chat(Server *srv, const char *name);
Chat* server_get_chat_fallback(Server *srv, const char *name);

Chat *chat_new(Server *srv, const char *name);
void chat_free(Chat *chat);
int chat_add_user(Chat *chat, const char *nick, UserType type);
int chat_add_user_full(Chat *chat, User *user);
int chat_rm_user(Chat *chat, const char *nick);
User* chat_get_user(Chat *chat, const char *nick);
void chat_add_sent_message(Chat *chat, const char *content);
void chat_add_recv_message(Chat *chat, const char *origin, const char *content);
void chat_add_action_message(Chat *chat, const char *origin, const char *content);
void chat_add_notice_message(Chat *chat, const char *origin, const char *content);
void chat_add_misc_message(Chat *chat, const char *origin, const char *content);
void chat_add_misc_message_fmt(Chat *chat, const char *origin, const char *fmt, ...);
void chat_add_error_message(Chat *chat, const char *origin, const char *content);
void chat_add_error_message_fmt(Chat *chat, const char *origin, const char *fmt, ...);
void chat_set_topic(Chat *chat, const char *origin, const char *topic);
void chat_set_topic_setter(Chat *chat, const char *setter);

ChatPrefs *chat_prefs_new();
SrnRet chat_prefs_check(ChatPrefs *prefs);
void chat_prefs_free(ChatPrefs *prefs);

User *user_new(Chat *chat, const char *nick, const char *username, const char *realname, UserType type);
User *user_ref(User *user);
void user_free(User *user);
void user_rename(User *user, const char *new_nick);
void user_set_type(User *user, UserType type);
void user_set_me(User *user, bool me);

Message* message_new(Chat *chat, User *user, const char *content, MessageType type);
void message_free(Message *msg);

ServerPrefs* server_prefs_new(const char *name);
ServerPrefs* server_prefs_new_from_basename(const char *basename);
ServerPrefs* server_prefs_get_prefs(const char *name);
ServerPrefs* server_prefs_get_prefs_by_host_port(const char *host, int port);
bool server_prefs_is_valid(ServerPrefs *prefs);
bool server_prefs_is_server_valid(Server *srv);
SrnRet server_prefs_check(ServerPrefs *prefs);
char* server_prefs_dump(ServerPrefs *prefs);
void server_prefs_free(ServerPrefs *prefs);
char* server_prefs_list_dump();
char* login_method_to_string(LoginMethod login);
LoginMethod login_method_from_string(const char *str);

ServerCap* server_cap_new();
void server_cap_free(ServerCap *scap);
SrnRet server_cap_server_enable(ServerCap *scap, const char *name, bool enable);
SrnRet server_cap_client_enable(ServerCap *scap, const char *name, bool enable);
bool server_cap_all_enabled(ServerCap *scap);
bool server_cap_is_support(ServerCap *scap, const char *name, const char *value);
char* server_cap_dump(ServerCap *scap);

SrnRet server_url_open(const char *url);

#endif /* __SERVER_H */
