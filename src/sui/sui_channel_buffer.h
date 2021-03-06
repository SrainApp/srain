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

#ifndef __SUI_CHANNEL_BUFFER_H
#define __SUI_CHANNEL_BUFFER_H

#include <gtk/gtk.h>

#include "sui/sui.h"

#define SUI_TYPE_CHANNEL_BUFFER (sui_channel_buffer_get_type())
#define SUI_CHANNEL_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_CHANNEL_BUFFER, SuiChannelBuffer))
#define SUI_IS_CHANNEL_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_CHANNEL_BUFFER))

typedef struct _SuiChannelBuffer SuiChannelBuffer;
typedef struct _SuiChannelBufferClass SuiChannelBufferClass;

GType sui_channel_buffer_get_type(void);
SuiChannelBuffer* sui_channel_buffer_new(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg);

#endif /* __SUI_CHANNEL_BUFFER_H */
