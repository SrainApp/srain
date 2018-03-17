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

#ifndef __SRAIN_SERVER_BUFFER_H
#define __SRAIN_SERVER_BUFFER_H

#include <gtk/gtk.h>
#include "sui/sui.h"

#define CHANNEL_LIST_STORE_COL_CHANNEL     0
#define CHANNEL_LIST_STORE_COL_USERS       1
#define CHANNEL_LIST_STORE_COL_TOPIC       2

#define SRAIN_TYPE_SERVER_BUFFER (srain_server_buffer_get_type())
#define SRAIN_SERVER_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_SERVER_BUFFER, SrainServerBuffer))
#define SRAIN_IS_SERVER_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_SERVER_BUFFER))

typedef struct _SrainServerBuffer SrainServerBuffer;
typedef struct _SrainServerBufferClass SrainServerBufferClass;

GType srain_server_buffer_get_type(void);
SrainServerBuffer* srain_server_buffer_new(const char *server, SuiBufferEvents *events, SuiBufferConfig *cfg);

void srain_server_buffer_add_buffer(SrainServerBuffer *self, SuiBuffer *buffer);
void srain_server_buffer_rm_buffer(SrainServerBuffer *self, SuiBuffer *buffer);
GSList* srain_server_buffer_get_buffer_list(SrainServerBuffer *self);
GtkTreeModelFilter* srain_server_buffer_get_channel_model(SrainServerBuffer *self);

bool srain_server_buffer_is_adding_channel(SrainServerBuffer *self);
void srain_server_buffer_start_add_channel(SrainServerBuffer *self);
void srain_server_buffer_add_channel(SrainServerBuffer *self, const char *chan, int users, const char *topic);
void srain_server_buffer_end_add_channel(SrainServerBuffer *self);

#endif /* __SRAIN_SERVER_BUFFER_H */
