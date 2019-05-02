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
typedef struct _SuiUser SuiUser;
typedef struct _SuiMessage SuiMessage;
typedef enum _SuiMiscMessageStyle SuiMiscMessageStyle;

enum _SuiMiscMessageStyle {
    SUI_MISC_MESSAGE_STYLE_NONE = 0,
    SUI_MISC_MESSAGE_STYLE_NORMAL,
    SUI_MISC_MESSAGE_STYLE_ERROR,
    SUI_MISC_MESSAGE_STYLE_ACTION,
    SUI_MISC_MESSAGE_STYLE_UNKNOWN,
};

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
SuiApplicationOptions* sui_application_get_options(SuiApplication *app);

/* SuiWindow */
SuiWindow* sui_new_window(SuiApplication *app, SuiWindowEvents *events);
void sui_free_window(SuiWindow *win);

/* SuiBuffer */
SuiBuffer* sui_new_buffer(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg);
void sui_free_buffer(SuiBuffer *buf);

void* sui_buffer_get_ctx(SuiBuffer *buf);
void sui_buffer_set_config(SuiBuffer *buf, SuiBufferConfig *cfg);
void sui_buffer_add_message(SuiBuffer *buf, SuiMessage *msg);

/* SuiMessage */
SuiMessage *sui_new_misc_message(void *ctx, SuiMiscMessageStyle style);
SuiMessage *sui_new_send_message(void *ctx);
SuiMessage *sui_new_recv_message(void *ctx);

void sui_update_message(SuiMessage *msg);
void sui_notify_message(SuiMessage *msg);

/* User */
SuiUser* sui_new_user(void *ctx);
void sui_free_user(SuiUser *user);
void sui_update_user(SuiUser *user);
void sui_add_user(SuiBuffer *buf, SuiUser *user);
void sui_rm_user(SuiBuffer *buf, SuiUser *user);

/* Misc */
void sui_set_topic(SuiBuffer *sui, const char *topic);
void sui_set_topic_setter(SuiBuffer *sui, const char *setter);
void sui_message_box(const char *title, const char *msg);

void sui_chan_list_start(SuiBuffer *sui);
void sui_chan_list_add(SuiBuffer *sui, const char *chan, int users, const char *topic);
void sui_chan_list_end(SuiBuffer *sui);

#endif /* __SUI_H */
