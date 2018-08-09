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

#ifndef __SUI_SERVER_BUFFER_H
#define __SUI_SERVER_BUFFER_H

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "sui_join_panel.h"

#define CHANNEL_LIST_STORE_COL_CHANNEL     0
#define CHANNEL_LIST_STORE_COL_USERS       1
#define CHANNEL_LIST_STORE_COL_TOPIC       2

#define SUI_TYPE_SERVER_BUFFER (sui_server_buffer_get_type())
#define SUI_SERVER_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_SERVER_BUFFER, SuiServerBuffer))
#define SUI_IS_SERVER_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_SERVER_BUFFER))

typedef struct _SuiServerBuffer SuiServerBuffer;
typedef struct _SuiServerBufferClass SuiServerBufferClass;

GType sui_server_buffer_get_type(void);
SuiServerBuffer* sui_server_buffer_new(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg);

void sui_server_buffer_add_buffer(SuiServerBuffer *self, SuiBuffer *buf);
void sui_server_buffer_rm_buffer(SuiServerBuffer *self, SuiBuffer *buf);
void sui_server_buffer_add_channel(SuiServerBuffer *self, const char *chan, int users, const char *topic);
void sui_server_buffer_clear_channel(SuiServerBuffer *self);

GSList* sui_server_buffer_get_buffer_list(SuiServerBuffer *self);
SuiJoinPanel* sui_server_buffer_get_join_panel(SuiServerBuffer *self);

#endif /* __SUI_SERVER_BUFFER_H */
