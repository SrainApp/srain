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
#define SRN_MESSAGE_MERGE_INTERVAL  60

typedef enum   _SrnMessageType SrnMessageType;
typedef struct _SrnMessage SrnMessage;
// typedef struct _UserType UserType;
typedef struct _SrnServerUser SrnServerUser;
typedef struct _SrnChatUser SrnChatUser;
typedef struct _SrnChat SrnChat;
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
    SRN_SERVER_USER_CHIGUA,    // No prefix
    SRN_SERVER_USER_OWNER,     // ~ mode +q
    SRN_SERVER_USER_ADMIN,     // & mode +a
    SRN_SERVER_USER_FULL_OP,   // @ mode +o
    SRN_SERVER_USER_HALF_OP,   // % mode +h
    SRN_SERVER_USER_VOICED,    // + mode +v
    SRN_SERVER_USER_TYPE_MAX
}; */

typedef struct _SrnUserContext SrnUserContext;

struct _SrnChatUser{
    SrnChat *chat;

    SrnChatUserType type;
    GList *msg_list;    // TODO: list of SrnMessage
    // SuiUser *ui;

    SrnServerUser *srv_user;
};

struct _SrnServerUser {
    SrnServer *srv;

    char *nick; // TODO: servername support
    char *username;
    char *hostname;
    char *realname;
    char *loginname;
    char *serverloc;

    bool is_me;
    bool is_server;
    bool is_online;
    bool is_away;
    bool is_secure;

    GSList *chat_user_list;  // List of SrnChatUser
};

enum _SrnMessageType {
    SRN_MESSAGE_UNKNOWN,
    SRN_MESSAGE_RECV,
    SRN_MESSAGE_SENT,
    SRN_MESSAGE_ACTION,
    SRN_MESSAGE_NOTICE,
    SRN_MESSAGE_MISC,
    SRN_MESSAGE_ERROR,
};

struct _SrnMessage {
    SrnChat *chat;
    SrnChatUser *user;  // Originator of this message, often refers to an existing user
    char *dname;    // Decorated name, maybe contains xml tags
    char *role;     // The role of the message originator

    char *content;
    char *dcontent; // Decorated message content
    time_t time;
    bool mentioned;
    SrnMessageType type;

    GSList *urls;   // URLs contains in message, like "http://xxx", "irc://xxx"
    SuiMessage *ui;
};

/* Represent a channel or dialog or a server session */
struct _SrnChat {
    char *name;
    bool joined;
    SrnChatUser *user;  // Yourself
    SrnChatUser *_user; // Hold all messages that do not belong other any user

    GSList *user_list;  // List of SrnChatUser
    GList *msg_list;
    SrnMessage *last_msg;

    /* Used by Filters & Decorators */
    GSList *ignore_nick_list;
    GSList *ignore_regex_list;
    GSList *relaybot_list;

    SrnServer *srv;
    SuiBuffer *ui;
    SrnChatConfig *cfg;
};

struct _SrnChatConfig {
    bool log; // TODO
    bool render_mirc_color;

    SuiBufferConfig *ui;
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
    SrnServerCap *cap;      // Server capabilities
    SrnServerConfig *cfg;   // All required static informations

    SrnServerUser *user;    // Used to store your nick, username, realname
    SrnServerUser *_user;   // Hold all messages that do not belong other any user
    SrnChat *chat;          // Hold all messages that do not belong to any other SrnChat
    SrnChat *cur_chat;
    GSList *chat_list;      // List of SrnChat
    GHashTable *user_table; // Hash table of SrnServerUser

    SircSession *irc;
};

enum _SrnLoginMethod {
    SRN_LOGIN_METHOD_NONE,
    SRN_LOGIN_METHOD_PASS,
    SRN_LOGIN_METHOD_NICKSERV,
    SRN_LOGIN_METHOD_MSG_NICKSERV,
    SRN_LOGIN_METHOD_SASL_PLAIN,
    SRN_LOGIN_METHOD_UNKNOWN,
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

    /* SrnServerUser */
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
void srn_server_free(SrnServer *srv);
void srn_server_set_config(SrnServer *srv, SrnServerConfig *cfg);
SrnRet srn_server_reload_config(SrnServer *srv);
bool srn_server_is_valid(SrnServer *srv);
SrnRet srn_server_connect(SrnServer *srv);
SrnRet srn_server_disconnect(SrnServer *srv);
SrnRet srn_server_state_transfrom(SrnServer *srv, SrnServerAction act);
bool srn_server_is_registered(SrnServer *srv);
void srn_server_wait_until_registered(SrnServer *srv);
int srn_server_add_chat(SrnServer *srv, const char *name);
SrnRet srn_server_rm_chat(SrnServer *srv, SrnChat *chat);
SrnChat* srn_server_get_chat(SrnServer *srv, const char *name);
SrnChat* srn_server_get_chat_fallback(SrnServer *srv, const char *name);
SrnRet srn_server_add_user(SrnServer *srv, const char *nick);
SrnRet srn_server_rm_user(SrnServer *srv, const char *nick);
SrnServerUser* srn_server_get_user(SrnServer *srv, const char *nick);
SrnServerUser* srn_server_add_and_get_user(SrnServer *srv, const char *nick);
SrnRet srn_server_rename_user(SrnServer *srv, const char *old_nick, const char *new_nick);

SrnChat* srn_chat_new(SrnServer *srv, const char *name, SrnChatConfig *cfg);
void srn_chat_free(SrnChat *chat);
void srn_chat_set_config(SrnChat *chat, SrnChatConfig *cfg);
SrnRet srn_chat_add_user(SrnChat *chat, SrnServerUser *srv_user);
SrnRet srn_chat_rm_user(SrnChat *chat, SrnChatUser *user);
SrnChatUser* srn_chat_get_user(SrnChat *chat, const char *nick);
SrnChatUser* srn_chat_add_and_get_user(SrnChat *chat, SrnServerUser *srv_user);
void srn_chat_add_sent_message(SrnChat *chat, const char *content);
void srn_chat_add_recv_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_action_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_notice_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_misc_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_misc_message_fmt(SrnChat *chat, SrnChatUser *user, const char *fmt, ...);
void srn_chat_add_error_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_error_message_fmt(SrnChat *chat, SrnChatUser *user, const char *fmt, ...);
void srn_chat_set_topic(SrnChat *chat, SrnChatUser *user, const char *topic);
void srn_chat_set_topic_setter(SrnChat *chat, const char *setter);

SrnChatConfig *srn_chat_config_new();
SrnRet srn_chat_config_check(SrnChatConfig *cfg);
void srn_chat_config_free(SrnChatConfig *cfg);

SrnServerUser *srn_server_user_new(SrnServer *srv, const char *nick);
SrnServerUser *srn_server_user_ref(SrnServerUser *user);
void srn_server_user_free(SrnServerUser *user);
void srn_server_user_set_nick(SrnServerUser *user, const char *nick);
void srn_server_user_set_username(SrnServerUser *user, const char *username);
void srn_server_user_set_hostname(SrnServerUser *user, const char *hostname);
void srn_server_user_set_realname(SrnServerUser *user, const char *realname);
void srn_server_user_set_me(SrnServerUser *user, bool me);
SrnRet srn_server_user_attach_chat_user(SrnServerUser *user, SrnChatUser *chat_user);
SrnRet srn_server_user_detach_chat_user(SrnServerUser *user, SrnChatUser *chat_user);

SrnChatUser *srn_chat_user_new(SrnChat *chat, SrnServerUser *srv_user);
void srn_chat_user_free(SrnChatUser *self);
void srn_chat_user_set_type(SrnChatUser *self, SrnChatUserType type);
void srn_chat_user_update(SrnChatUser *self);

SrnMessage* srn_message_new(SrnChat *chat, SrnChatUser *user, const char *content, SrnMessageType type);
void srn_message_free(SrnMessage *msg);

SrnServerConfig* srn_server_config_new(const char *name);
SrnRet srn_server_config_add_addr(SrnServerConfig *cfg, const char *addr);
SrnRet srn_server_config_set_addr(SrnServerConfig *cfg, const char *addr);
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

#endif /* __SERVER_H */
