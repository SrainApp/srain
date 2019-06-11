/* Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __CHAT_H
#define __CHAT_H

#include <glib.h>

#include "sirc/sirc.h"
#include "sui/sui.h"
#include "ret.h"
#include "extra_data.h"

#ifndef __IN_CORE_H
	#error This file should not be included directly, include just core.h
#endif

typedef struct _SrnChat SrnChat;
typedef enum   _SrnChatType SrnChatType;
typedef struct _SrnChatConfig SrnChatConfig;
typedef struct _SrnChatUser SrnChatUser;
typedef enum   _SrnChatUserType SrnChatUserType;

#include "./server.h"
#include "./message.h"

enum _SrnChatType {
    SRN_CHAT_TYPE_SERVER,
    SRN_CHAT_TYPE_CHANNEL,
    SRN_CHAT_TYPE_DIALOG,
};

/* Represent a channel or dialog or a server session */
struct _SrnChat {
    char *name;
    SrnChatType type;
    bool is_joined;

    SrnChatUser *user;  // Yourself
    SrnChatUser *_user; // Hold all messages that do not belong other any user
    GList *user_list;  // List of SrnChatUser

    GList *msg_list;
    SrnMessage *last_msg;

    /* Used by Filters & Decorators */
    GList *ignore_regex_list;
    GList *relaybot_list;

    SrnServer *srv;
    SuiBuffer *ui;
    SrnChatConfig *cfg;

    SrnExtraData *extra_data;
};

struct _SrnChatConfig {
    bool log; // TODO
    bool render_mirc_color;
    char *password;
    GList *auto_run_cmd_list;

    SuiBufferConfig *ui;
};

enum _SrnChatUserType {
    SRN_CHAT_USER_TYPE_OWNER,     // ~ mode +q
    SRN_CHAT_USER_TYPE_ADMIN,     // & mode +a
    SRN_CHAT_USER_TYPE_FULL_OP,   // @ mode +o
    SRN_CHAT_USER_TYPE_HALF_OP,   // % mode +h
    SRN_CHAT_USER_TYPE_VOICED,    // + mode +v
    SRN_CHAT_USER_TYPE_CHIGUA,    // No prefix
    /* ... */
    SRN_SERVER_USER_TYPE_MAX
};

struct _SrnChatUser{
    SrnChat *chat;

    bool is_joined;
    bool is_ignored;            // TODO: New implementation of ignore list

    SrnChatUserType type;
    SrnServerUser *srv_user;
    GList *msg_list;    // TODO: List of SrnMessage

    SuiUser *ui;

    SrnExtraData *extra_data;
};

SrnChat* srn_chat_new(SrnServer *srv, const char *name, SrnChatType type, SrnChatConfig *cfg);
void srn_chat_free(SrnChat *chat);
void srn_chat_set_config(SrnChat *chat, SrnChatConfig *cfg);
void srn_chat_set_is_joined(SrnChat *chat, bool joined);
SrnRet srn_chat_run_command(SrnChat *chat, const char *cmd);
GList* srn_chat_complete_command(SrnChat *chat, const char *cmd);
SrnRet srn_chat_add_user(SrnChat *chat, SrnServerUser *srv_user);
SrnRet srn_chat_rm_user(SrnChat *chat, SrnChatUser *user);
SrnChatUser* srn_chat_get_user(SrnChat *chat, const char *nick);
SrnChatUser* srn_chat_add_and_get_user(SrnChat *chat, SrnServerUser *srv_user);
void srn_chat_add_sent_message(SrnChat *chat, const char *content); void srn_chat_add_recv_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_action_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_notice_message(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_misc_message(SrnChat *self, const char *content);
void srn_chat_add_misc_message_fmt(SrnChat *self, const char *fmt, ...);
void srn_chat_add_misc_message_with_user(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_misc_message_with_user_fmt(SrnChat *chat, SrnChatUser *user, const char *fmt, ...);
void srn_chat_add_error_message(SrnChat *self, const char *content);
void srn_chat_add_error_message_fmt(SrnChat *self, const char *fmt, ...);
void srn_chat_add_error_message_with_user(SrnChat *chat, SrnChatUser *user, const char *content);
void srn_chat_add_error_message_with_user_fmt(SrnChat *chat, SrnChatUser *user, const char *fmt, ...);
void srn_chat_set_topic(SrnChat *chat, SrnChatUser *user, const char *topic);
void srn_chat_set_topic_setter(SrnChat *chat, const char *setter);

SrnChatConfig *srn_chat_config_new();
void srn_chat_config_free(SrnChatConfig *cfg);
SrnRet srn_chat_config_check(SrnChatConfig *cfg);

SrnChatUser *srn_chat_user_new(SrnChat *chat, SrnServerUser *srv_user);
void srn_chat_user_free(SrnChatUser *self);
void srn_chat_user_update(SrnChatUser *self);
void srn_chat_user_set_type(SrnChatUser *self, SrnChatUserType type);
void srn_chat_user_set_is_joined(SrnChatUser *self, bool joined);
void srn_chat_user_set_is_ignored(SrnChatUser *self, bool ignored);

#endif /* __CHAT_H */
