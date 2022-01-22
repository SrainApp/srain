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
#include "extra_data.h"

#ifndef __IN_CORE_H
	#error This file should not be included directly, include just core.h
#endif

/* In millseconds */
#define SRN_SERVER_PING_INTERVAL    (30 * 1000)
#define SRN_SERVER_PING_TIMEOUT     (SRN_SERVER_PING_INTERVAL * 2)
#define SRN_SERVER_RECONN_INTERVAL  (5 * 1000)
#define SRN_SERVER_RECONN_STEP      SRN_SERVER_RECONN_INTERVAL

typedef struct _SrnServerUser SrnServerUser;
typedef struct _SrnServerAddr SrnServerAddr;
typedef enum   _SrnServerState SrnServerState;
typedef enum   _SrnServerAction SrnServerAction;
typedef struct _SrnServer SrnServer;
typedef enum   _SrnLoginMethod SrnLoginMethod;
typedef struct _SrnLoginConfig SrnLoginConfig;
typedef struct _SrnUserConfig SrnUserConfig;
typedef struct _SrnServerConfig SrnServerConfig;
typedef struct _EnabledCap EnabledCap;
typedef struct _SrnServerCap SrnServerCap;

#include "chat.h"

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
    bool is_ignored;
    bool is_away;
    bool is_secure;

    GList *chat_user_list;  // List of SrnChatUser

    SrnExtraData *extra_data;
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
    /* Meta info */
    char *name;
    SrnServerConfig *cfg;    // All required static informations
    SrnServerAddr *addr;     // Current server addr, is a element of
                             // SrnServerConfig->addrs
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

    SrnServerCap *cap;      // Server capabilities

    SrnServerUser *user;    // Used to store your nick, username, realname
    SrnServerUser *_user;   // Hold all messages that do not belong other any user
    SrnChat *chat;          // Hold all messages that do not belong to any other SrnChat
    SrnChat *cur_chat;
    GList *chat_list;      // List of SrnChat
    GHashTable *user_table; // Hash table of SrnServerUser

    SircSession *irc; // IRC session
};

enum _SrnLoginMethod {
    SRN_LOGIN_METHOD_NONE,
    SRN_LOGIN_METHOD_NICKSERV,
    SRN_LOGIN_METHOD_MSG_NICKSERV,
    SRN_LOGIN_METHOD_SASL_PLAIN,
    SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE,
    SRN_LOGIN_METHOD_SASL_EXTERNAL,
    SRN_LOGIN_METHOD_UNKNOWN,
};

struct _SrnLoginConfig {
    SrnLoginMethod method;

    char *password;
    char *cert_file;
    // ...
};

struct _SrnUserConfig {
    char *nick;
    char *username;
    char *realname;

    char *away_message;
    char *part_message;
    char *kick_message;
    char *quit_message;

    SrnLoginConfig *login;
};

struct _SrnServerAddr {
    char *host;
    int port;
};

struct _SrnServerConfig {
    char *name;
    GList *addrs; // List of SrnServerAddr
    char *password;
    GList *auto_join_chat_list;
    GList *auto_run_cmd_list; // List of autorun commands

    /* SrnServerUser */
    SrnUserConfig *user;
    SircConfig *irc;
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
    bool message_tags;
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

SrnServer* srn_server_new(const char *name, SrnServerConfig *cfg);
void srn_server_free(SrnServer *srv);
SrnRet srn_server_quit(SrnServer *srv, const char *reason);
void srn_server_set_config(SrnServer *srv, SrnServerConfig *cfg);
SrnRet srn_server_reload_config(SrnServer *srv);
bool srn_server_is_valid(SrnServer *srv);
bool srn_server_is_chat_valid(SrnServer *srv, SrnChat *chat);
SrnRet srn_server_connect(SrnServer *srv);
SrnRet srn_server_disconnect(SrnServer *srv);
SrnRet srn_server_reconnect(SrnServer *srv);
SrnRet srn_server_state_transfrom(SrnServer *srv, SrnServerAction act);
bool srn_server_is_registered(SrnServer *srv);
void srn_server_wait_until_registered(SrnServer *srv);
int srn_server_add_chat(SrnServer *srv, const char *name);
SrnRet srn_server_rm_chat(SrnServer *srv, SrnChat *chat);
SrnChat* srn_server_get_chat(SrnServer *srv, const char *name);
SrnChat* srn_server_get_chat_fallback(SrnServer *srv, const char *name);
SrnChat* srn_server_add_and_get_chat(SrnServer *srv, const char *name);
SrnRet srn_server_add_user(SrnServer *srv, const char *nick);
SrnRet srn_server_rm_user(SrnServer *srv, SrnServerUser *user);
SrnServerUser* srn_server_get_user(SrnServer *srv, const char *nick);
SrnServerUser* srn_server_add_and_get_user(SrnServer *srv, const char *nick);
SrnRet srn_server_rename_user(SrnServer *srv, SrnServerUser *user, const char *nick);

SrnServerUser *srn_server_user_new(SrnServer *srv, const char *nick);
SrnServerUser *srn_server_user_ref(SrnServerUser *user);
void srn_server_user_free(SrnServerUser *user);
void srn_server_user_set_nick(SrnServerUser *user, const char *nick);
void srn_server_user_set_username(SrnServerUser *user, const char *username);
void srn_server_user_set_hostname(SrnServerUser *user, const char *hostname);
void srn_server_user_set_realname(SrnServerUser *user, const char *realname);
void srn_server_user_set_is_me(SrnServerUser *user, bool me);
void srn_server_user_set_is_online(SrnServerUser *user, bool online);
void srn_server_user_set_is_ignored(SrnServerUser *user, bool ignored);
SrnRet srn_server_user_attach_chat_user(SrnServerUser *user, SrnChatUser *chat_user);
SrnRet srn_server_user_detach_chat_user(SrnServerUser *user, SrnChatUser *chat_user);

SrnServerConfig* srn_server_config_new();
SrnRet srn_server_config_check(SrnServerConfig *cfg);
char* srn_server_config_dump(SrnServerConfig *cfg);
void srn_server_config_free(SrnServerConfig *cfg);
SrnRet srn_server_config_add_addr(SrnServerConfig *cfg, SrnServerAddr *addr);
void srn_server_config_clear_addr(SrnServerConfig *cfg);

SrnUserConfig* srn_user_config_new(void);
void srn_user_config_free(SrnUserConfig *self);
SrnRet srn_user_config_check(SrnUserConfig *self);
char* srn_user_config_get_next_alternate_nick(SrnUserConfig *self, const char *cur_nick);
bool srn_user_config_is_alternate_nick(SrnUserConfig *self, const char *nick);

SrnLoginConfig* srn_login_config_new(void);
void srn_login_config_free(SrnLoginConfig *self);
SrnRet srn_login_config_check(SrnLoginConfig *self);
const char* srn_login_method_to_string(SrnLoginMethod login);
SrnLoginMethod srn_login_method_from_string(const char *str);

SrnServerAddr* srn_server_addr_new(const char *host, int port);
SrnServerAddr* srn_server_addr_new_from_string(const char *str);
void  srn_server_addr_free(SrnServerAddr *addr);
bool srn_server_addr_equal(SrnServerAddr *addr1, SrnServerAddr *addr2);

SrnServerCap* srn_server_cap_new();
void srn_server_cap_free(SrnServerCap *scap);
SrnRet srn_server_cap_server_enable(SrnServerCap *scap, const char *name, bool enable);
SrnRet srn_server_cap_client_enable(SrnServerCap *scap, const char *name, bool enable);
bool srn_server_cap_all_enabled(SrnServerCap *scap);
bool srn_server_cap_is_support(SrnServerCap *scap, const char *name, const char *value);
char* srn_server_cap_dump(SrnServerCap *scap);

#endif /* __SERVER_H */
