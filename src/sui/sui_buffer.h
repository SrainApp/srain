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

#ifndef __SUI_BUFFER_H
#define __SUI_BUFFER_H

#include <gtk/gtk.h>

#include "sui_message.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_entry_completion.h"

#define SUI_TYPE_BUFFER (sui_buffer_get_type())
#define SUI_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_BUFFER, SuiBuffer))
#define SUI_IS_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_BUFFER))

typedef struct _SuiBuffer SuiBuffer;
typedef struct _SuiBufferClass SuiBufferClass;

GType sui_buffer_get_type(void);
SuiBuffer* sui_buffer_new(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg);

void sui_buffer_insert_text(SuiBuffer *self, const char *text, int line, int offset);
void sui_buffer_show_topic(SuiBuffer *self, bool show);
void sui_buffer_show_user_list(SuiBuffer *self, bool show);

void* sui_buffer_get_ctx(SuiBuffer *self);
SuiBufferEvents* sui_buffer_get_events(SuiBuffer *self);
void sui_buffer_set_config(SuiBuffer *self, SuiBufferConfig *cfg);
SuiBufferConfig* sui_buffer_get_config(SuiBuffer *self);
const char* sui_buffer_get_name(SuiBuffer *self);
const char* sui_buffer_get_remark(SuiBuffer *self);
void sui_buffer_set_topic(SuiBuffer *self, const char *topic);
void sui_buffer_set_topic_setter(SuiBuffer *self, const char *setter);
GtkMenu* sui_buffer_get_menu(SuiBuffer *self);
SrainMsgList* sui_buffer_get_msg_list(SuiBuffer *self);
SrainUserList* sui_buffer_get_user_list(SuiBuffer *self);
GtkTextBuffer* sui_buffer_get_input_text_buffer(SuiBuffer *self);

#endif /* __SUI_BUFFER_H */
