
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

#ifndef __SRAIN_PRIVATE_BUFFER_H
#define __SRAIN_PRIVATE_BUFFER_H

#include <gtk/gtk.h>
#include "sui/sui.h"
#include "srain_server_buffer.h"

#define SRAIN_TYPE_PRIVATE_BUFFER (srain_private_buffer_get_type())
#define SRAIN_PRIVATE_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_PRIVATE_BUFFER, SrainPrivateBuffer))
#define SRAIN_IS_PRIVATE_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_PRIVATE_BUFFER))

typedef struct _SrainPrivateBuffer SrainPrivateBuffer;
typedef struct _SrainPrivateBufferClass SrainPrivateBufferClass;

GType srain_private_buffer_get_type(void);
SrainPrivateBuffer* srain_private_buffer_new(SrainServerBuffer *srv,
        const char *nick, void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg);

void srain_private_buffer_show_user_list(SrainPrivateBuffer *self, bool isshow);

#endif /* __SRAIN_PRIVATE_BUFFER_H */
