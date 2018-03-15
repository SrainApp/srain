/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
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
#include "ret.h"

#ifndef __IN_CORE_H
	#error This file should not be included directly, include just core.h
#endif

/* In millseconds */
#define SRN_SERVER_PING_INTERVAL    (30 * 1000)
#define SRN_SERVER_PING_TIMEOUT     (SRN_SERVER_PING_INTERVAL * 2)
#define SRN_SERVER_RECONN_INTERVAL  (5 * 1000)
#define SRN_SERVER_RECONN_STEP      SRN_SERVER_RECONN_INTERVAL

/* In seconds */
#define MESSAGE_MERGE_INTERVAL  60

/* Structure members length */
#define NICK_LEN        128

typedef enum   _MessageType MessageType;
typedef struct _Message Message;
// typedef struct _UserType UserType;
typedef struct _User User;
typedef struct _Chat Chat;
typedef struct _SrnChatConfig SrnChatConfig;
typedef struct _SrnServerAddr SrnServerAddr;
typedef enum   _SrnServerState SrnServerState;
typedef enum   _SrnServerAction SrnServerAction;
typedef struct _SrnServer SrnServer;
typedef enum   _SrnLoginMethod SrnLoginMethod;
typedef struct _SrnServerConfig SrnServerConfig;
typedef struct _EnabledCap EnabledCap;
typedef struct _SrnServerCap SrnServerCap;


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
    char *name;
    bool joined;
    User *user;         // Yourself

    GSList *user_list;
    GList *msg_list;
    Message *last_msg;

    /* Used by Filters & Decorators */
    GSList *ignore_nick_list;
    GSList *ignore_regex_list;
    GSList *relaybot_list;

    SrnServer *srv;
    SuiSession *ui;
    SrnChatConfig *cfg;
};

struct _SrnChatConfig {
    bool log; // TODO
    bool render_mirc_color;

    SuiConfig *ui;
};

enum _SrnServerState {
    SRN_SERVER_STATE_DISCONNECTED,
    SRN_SERVER_STATE_CONNECTING,
    SRN_SERVER_STATE_CONNECTED,
    SRN_SERVER_STATE_DISCONNECTING,
    SRN_SERVER_STATE_QUITING,
    SRN_SERVER_STATE_RECONNECTING,
};

enum _SrnServerAction {
    SRN_SERVER_ACTION_CONNECT,
    SRN_SERVER_ACTION_CONNECT_FAIL,
    SRN_SERVER_ACTION_CONNECT_FINISH,
    SRN_SERVER_ACTION_DISCONNECT,
    SRN_SERVER_ACTION_QUIT,
    SRN_SERVER_ACTION_RECONNECT,
    SRN_SERVER_ACTION_DISCONNECT_FINISH,
};

struct _SrnServer {
    /* Status */
    SrnServerState state;
    SrnServerAction last_action;
    bool negotiated;    // Client capability negotiation has finished
    bool registered;    // User has a nickname
    bool loggedin;      // User has identified as a certain account

    /* Keep alive */
    unsigned long last_pong;        // Last pong time, in ms
    unsigned long delay;            // Delay in ms
    unsigned long reconn_interval;  // Interval of next reconnect, in ms
    int ping_timer;
    int reconn_timer;

    SrnServerAddr *addr;    // Current server addr, is a element of
                            // SrnServerConfig->addrs
    SrnServerCap *cap;         // Server capabilities
    SrnServerConfig *cfg;   // All required static informations
    User *user;             // Used to store your nick, username, realname
    Chat *chat;             // Hold all messages that do not belong to any other Chat

    Chat *cur_chat;
    GSList *chat_list;

    SircSession *irc;
};

enum _SrnLoginMethod {
    LOGIN_NONE,
    LOGIN_PASS,
    LOGIN_NICKSERV,
    LOGIN_MSG_NICKSERV,
    LOGIN_SASL_PLAIN,
    LOGIN_UNKNOWN,
};

struct _SrnServerAddr {
    char *host;
    int port;
};

struct _SrnServerConfig {
    /* For specificed server */
    bool predefined;  /* A SrnServerConfig is predefined when it is loaded from
                         configuration file, a predefined SrnServerConfig will
                         appeared in predefined server list and *CAN NOT* be
                         directly freed by ``srn_server_config_free()``. */
    char *name;
    GSList *addrs; // List of SrnServerAddr
    char *passwd;

    /* User */
    char *nickname;
    char *username;
    char *realname;
    char *user_passwd;
    SrnLoginMethod login_method;

    /* Default message */
    char *part_message;
    char *kick_message;
    char *away_message;
    char *quit_message;

    SircConfig *irc;

    // ...
    SrnServer *srv;
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

struct _SrnServerCap {
    /* Capabilities */
    EnabledCap client_enabled;
    EnabledCap server_enabled;

    SrnServer *srv;
};

SrnServer* srn_server_new(SrnServerConfig *cfg);
SrnServer *srn_server_get_by_name(const char *name);
void srn_server_free(SrnServer *srv);
bool srn_server_is_valid(SrnServer *srv);
SrnRet srn_server_connect(SrnServer *srv);
SrnRet srn_server_disconnect(SrnServer *srv);
SrnRet srn_server_state_transfrom(SrnServer *srv, SrnServerAction act);
bool srn_server_is_registered(SrnServer *srv);
void srn_server_wait_until_registered(SrnServer *srv);
int srn_server_add_chat(SrnServer *srv, const char *name);
SrnRet srn_server_rm_chat(SrnServer *srv, Chat *chat);
Chat* srn_server_get_chat(SrnServer *srv, const char *name);
Chat* srn_server_get_chat_fallback(SrnServer *srv, const char *name);

Chat *chat_new(SrnServer *srv, const char *name, SrnChatConfig *cfg);
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

SrnChatConfig *srn_chat_config_new();
SrnRet srn_chat_config_check(SrnChatConfig *cfg);
void srn_chat_config_free(SrnChatConfig *cfg);

User *user_new(Chat *chat, const char *nick, const char *username, const char *realname, UserType type);
User *user_ref(User *user);
void user_free(User *user);
void user_rename(User *user, const char *new_nick);
void user_set_type(User *user, UserType type);
void user_set_me(User *user, bool me);

Message* message_new(Chat *chat, User *user, const char *content, MessageType type);
void message_free(Message *msg);

SrnServerConfig* srn_server_config_new(const char *name);
SrnServerConfig* srn_server_config_new_from_basename(const char *basename);
void srn_server_config_add_addr(SrnServerConfig *cfg, const char *host, int port);
SrnRet srn_server_config_check(SrnServerConfig *cfg);
char* srn_server_config_dump(SrnServerConfig *cfg);
void srn_server_config_free(SrnServerConfig *cfg);

char* srn_login_method_to_string(SrnLoginMethod login);
SrnLoginMethod srn_login_method_from_string(const char *str);

SrnServerAddr* srn_server_addr_new(const char *host, int port);
void  srn_server_addr_free(SrnServerAddr *addr);

SrnServerCap* srn_server_cap_new();
void srn_server_cap_free(SrnServerCap *scap);
SrnRet srn_server_cap_server_enable(SrnServerCap *scap, const char *name, bool enable);
SrnRet srn_server_cap_client_enable(SrnServerCap *scap, const char *name, bool enable);
bool srn_server_cap_all_enabled(SrnServerCap *scap);
bool srn_server_cap_is_support(SrnServerCap *scap, const char *name, const char *value);
char* srn_server_cap_dump(SrnServerCap *scap);

SrnRet srn_server_url_open(const char *url);

SrnRet srn_server_cmd(Chat *chat, const char *cmd); // FIXME
#endif /* __SERVER_H */
