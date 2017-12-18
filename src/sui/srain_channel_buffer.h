/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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

#ifndef __SRAIN_CHANNEL_BUFFER_H
#define __SRAIN_CHANNEL_BUFFER_H

#include <gtk/gtk.h>
#include "sui/sui.h"
#include "srain_server_buffer.h"

#define SRAIN_TYPE_CHANNEL_BUFFER (srain_channel_buffer_get_type())
#define SRAIN_CHANNEL_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHANNEL_BUFFER, SrainChannelBuffer))
#define SRAIN_IS_CHANNEL_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHANNEL_BUFFER))

typedef struct _SrainChannelBuffer SrainChannelBuffer;
typedef struct _SrainChannelBufferClass SrainChannelBufferClass;

GType srain_channel_buffer_get_type(void);
SrainChannelBuffer* srain__buffer_new(SuiSession *sui, SrainServerBuffer *buffer, const char *chan);

void srain_channel_buffer_show_user_list(SrainChannelBuffer *self, bool isshow);

#endif /* __SRAIN_CHANNEL_BUFFER_H */
