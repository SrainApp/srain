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

#ifndef __SRAIN_CHAT_BUFFER_H
#define __SRAIN_CHAT_BUFFER_H

#include <gtk/gtk.h>
#include "sui/sui.h"
#include "srain_server_buffer.h"

#define SRAIN_TYPE_CHAT_BUFFER (srain_chat_buffer_get_type())
#define SRAIN_CHAT_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAT_BUFFER, SrainChatBuffer))
#define SRAIN_IS_CHAT_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAT_BUFFER))

struct _SrainChatBuffer {
    SuiBuffer parent;

    GtkCheckMenuItem *user_list_menu_item;

    SrainServerBuffer *server_buffer;
    SrainUserList *user_list;
};

struct _SrainChatBufferClass {
    SuiBufferClass parent_class;
};

typedef struct _SrainChatBuffer SrainChatBuffer;
typedef struct _SrainChatBufferClass SrainChatBufferClass;

GType srain_chat_buffer_get_type(void);

void srain_chat_buffer_show_user_list(SrainChatBuffer *self, bool isshow);
SrainServerBuffer* srain_chat_buffer_get_server_buffer(SrainChatBuffer *self);
SrainUserList* srain_chat_buffer_get_user_list(SrainChatBuffer *self);

#endif /* __SRAIN_CHAT_BUFFER_H */
