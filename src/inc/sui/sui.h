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

#ifndef __SUI_H
#define __SUI_H

#include <time.h>
#include "srain.h"

typedef struct _SuiApplication SuiApplication;
typedef struct _SuiWindow SuiWindow;
typedef struct _SuiBuffer SuiBuffer;
typedef int SuiBufferFlag;
typedef struct _SuiMessage SuiMessage;
typedef enum _UserType UserType;
typedef enum _SrnChatUserType SrnChatUserType;

// TODO Rename type
typedef enum {
    SYS_MSG_NORMAL,
    SYS_MSG_ERROR,
    SYS_MSG_ACTION
} SysMsgType;


enum _SrnChatUserType {
    SRN_SERVER_USER_OWNER,     // ~ mode +q
    SRN_SERVER_USER_ADMIN,     // & mode +a
    SRN_SERVER_USER_FULL_OP,   // @ mode +o
    SRN_SERVER_USER_HALF_OP,   // % mode +h
    SRN_SERVER_USER_VOICED,    // + mode +v
    SRN_SERVER_USER_CHIGUA,    // No prefix
    /* ... */
    SRN_SERVER_USER_TYPE_MAX
};

enum _UserType {
    USER_OWNER,     // ~ mode +q
    USER_ADMIN,     // & mode +a
    USER_FULL_OP,   // @ mode +o
    USER_HALF_OP,   // % mode +h
    USER_VOICED,    // + mode +v
    USER_CHIGUA,    // No prefix
    /* ... */
    USER_TYPE_MAX
};

#define SRAIN_MSG_MENTIONED 0x1

#define __IN_SUI_H
#include "sui_event.h"
#include "sui_config.h"
#undef __IN_SUI_H

void sui_proc_pending_event(void);

/* SuiAppliaction */
SuiApplication* sui_new_application(const char *id, void *ctx, SuiApplicationEvents *events, SuiApplicationConfig *cfg);
void sui_free_application(SuiApplication *app);
void sui_run_application(SuiApplication *app, int argc, char *argv[]);

void* sui_application_get_ctx(SuiApplication *app);

void sui_application_set_config(SuiApplication *app, SuiApplicationConfig *cfg);
SuiApplicationConfig* sui_application_get_config(SuiApplication *app);

/* SuiWindow */
SuiWindow* sui_new_window(SuiApplication *app, SuiWindowEvents *events, SuiWindowConfig *cfg);
void sui_free_window(SuiWindow *win);

/* SuiBuffer */
SuiBuffer* sui_new_buffer(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg);
void sui_free_buffer(SuiBuffer *buf);

void* sui_buffer_get_ctx(SuiBuffer *buf);
void sui_buffer_set_config(SuiBuffer *buf, SuiBufferConfig *cfg);

/* SuiMessage */

void sui_message_set_ctx(SuiMessage *smsg, void *ctx);
void *sui_message_get_ctx(SuiMessage *smsg);
void sui_message_set_time(SuiMessage *smsg, time_t time);
void sui_message_mentioned(SuiMessage *smsg);
void sui_message_notify(SuiMessage *smsg);

SuiMessage *sui_add_sys_msg(SuiBuffer *sui, const char *msg, SysMsgType type);
SuiMessage *sui_add_sent_msg(SuiBuffer *sui, const char *msg);
SuiMessage *sui_add_recv_msg(SuiBuffer *sui, const char *nick, const char *id, const char *msg);
void sui_message_append_message(SuiBuffer *sui, SuiMessage *smsg, const char *msg);

/* Completion */
void sui_add_completion(SuiBuffer *sui, const char *word);
void sui_rm_completion(SuiBuffer *sui, const char *word);

/* User */
SrnRet sui_add_user(SuiBuffer *sui, const char *nick, UserType type);
SrnRet sui_rm_user(SuiBuffer *sui, const char *nick);
SrnRet sui_ren_user(SuiBuffer *sui, const char *old_nick, const char *new_nick, UserType type);

/* Misc */
void sui_set_topic(SuiBuffer *sui, const char *topic);
void sui_set_topic_setter(SuiBuffer *sui, const char *setter);
void sui_message_box(const char *title, const char *msg);

void sui_chan_list_start(SuiBuffer *sui);
void sui_chan_list_add(SuiBuffer *sui, const char *chan, int users, const char *topic);
void sui_chan_list_end(SuiBuffer *sui);
void sui_server_list_add(const char *server);

#endif /* __SUI_H */
