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

#ifndef __SUI_CHAT_BUFFER_H
#define __SUI_CHAT_BUFFER_H

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "sui_server_buffer.h"
#include "sui_user_list.h"

#define SUI_TYPE_CHAT_BUFFER (sui_chat_buffer_get_type())
#define SUI_CHAT_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_CHAT_BUFFER, SuiChatBuffer))
#define SUI_IS_CHAT_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_CHAT_BUFFER))

struct _SuiChatBuffer {
    SuiBuffer parent;

    SuiServerBuffer *server_buffer;

    void *user_list_menu_item;
    GtkRevealer *user_list_revealer;
    SuiUserList *user_list;
};

struct _SuiChatBufferClass {
    SuiBufferClass parent_class;
};

typedef struct _SuiChatBuffer SuiChatBuffer;
typedef struct _SuiChatBufferClass SuiChatBufferClass;

GType sui_chat_buffer_get_type(void);

void sui_chat_buffer_show_user_list(SuiChatBuffer *self, bool isshow);
SuiServerBuffer* sui_chat_buffer_get_server_buffer(SuiChatBuffer *self);
SuiUserList* sui_chat_buffer_get_user_list(SuiChatBuffer *self);

#endif /* __SUI_CHAT_BUFFER_H */
