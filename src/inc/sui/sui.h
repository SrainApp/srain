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
typedef struct _SuiSession SuiSession;
typedef int SuiSessionFlag;
typedef struct _SuiMessage SuiMessage;
typedef enum _UserType UserType;
typedef enum _SrnUserType SrnUserType;

#define SUI_SESSION_SERVER      1 << 0
#define SUI_SESSION_CHANNEL     1 << 1
#define SUI_SESSION_DIALOG      1 << 2

// TODO Rename type
typedef enum {
    SYS_MSG_NORMAL,
    SYS_MSG_ERROR,
    SYS_MSG_ACTION
} SysMsgType;


enum _SrnUserType {
    SRN_USER_OWNER,     // ~ mode +q
    SRN_USER_ADMIN,     // & mode +a
    SRN_USER_FULL_OP,   // @ mode +o
    SRN_USER_HALF_OP,   // % mode +h
    SRN_USER_VOICED,    // + mode +v
    SRN_USER_CHIGUA,    // No prefix
    /* ... */
    SRN_USER_TYPE_MAX
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
SuiApplication* sui_new_application(const char *id, SuiApplicationEvents *events, SuiApplicationConfig *cfg);
void sui_free_application(SuiApplication *app);
SuiApplicationEvents* sui_application_get_events(SuiApplication *app);
void* sui_application_get_ctx(SuiApplication *app);
void sui_application_set_ctx(SuiApplication *app, void *ctx);
void sui_run_application(SuiApplication *app, int argc, char *argv[]);

/* SuiWindow */
SuiWindow* sui_new_window(SuiApplication *app, SuiWindowEvents *events, SuiWindowConfig *cfg);
void sui_free_window(SuiWindow *win);
SuiWindowEvents* sui_window_get_events(SuiWindow *sui);

/* SuiSession */
SuiSession *sui_new_session(SuiEvents *events, SuiConfig *cfg, SuiSessionFlag flag);
void sui_free_session(SuiSession *sui);
SrnRet sui_server_session(SuiSession *sui, const char *srv);
SrnRet sui_channel_session(SuiSession *sui, SuiSession *sui_srv, const char *chan);
SrnRet sui_private_session(SuiSession *sui, SuiSession *sui_srv, const char *nick);
void sui_end_session(SuiSession *sui);

SuiSessionFlag sui_get_flag(SuiSession *sui);
SuiEvents *sui_get_events(SuiSession *sui);
void* sui_get_ctx(SuiSession *sui);
void sui_set_ctx(SuiSession *sui, void *ctx);
void sui_set_name(SuiSession *sui, const char *name);
void sui_set_remark(SuiSession *sui, const char *remark);

/* SuiMessage */

void sui_message_set_ctx(SuiMessage *smsg, void *ctx);
void *sui_message_get_ctx(SuiMessage *smsg);
// TODO: pass a image file path
void sui_message_set_time(SuiMessage *smsg, time_t time);
void sui_message_append_image(SuiMessage *smsg, const char *url);
void sui_message_mentioned(SuiMessage *smsg);
void sui_message_notify(SuiMessage *smsg);

SuiMessage *sui_add_sys_msg(SuiSession *sui, const char *msg, SysMsgType type);
SuiMessage *sui_add_sent_msg(SuiSession *sui, const char *msg);
SuiMessage *sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id, const char *msg);
void sui_message_append_message(SuiSession *sui, SuiMessage *smsg, const char *msg);

/* Completion */
void sui_add_completion(SuiSession *sui, const char *word);
void sui_rm_completion(SuiSession *sui, const char *word);

/* User */
SrnRet sui_add_user(SuiSession *sui, const char *nick, UserType type);
SrnRet sui_rm_user(SuiSession *sui, const char *nick);
SrnRet sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick, UserType type);

/* Misc */
void sui_set_topic(SuiSession *sui, const char *topic);
void sui_set_topic_setter(SuiSession *sui, const char *setter);
void sui_message_box(const char *title, const char *msg);

void sui_chan_list_start(SuiSession *sui);
void sui_chan_list_add(SuiSession *sui, const char *chan, int users, const char *topic);
void sui_chan_list_end(SuiSession *sui);
void sui_server_list_add(const char *server);

#endif /* __SUI_H */
