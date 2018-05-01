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

#ifndef __SUI_MESSAGE_LIST_H
#define __SUI_MESSAGE_LIST_H

#include <gtk/gtk.h>
#include "sui_message.h"

#define SUI_TYPE_MESSAGE_LIST (sui_message_list_get_type())
#define SUI_MESSAGE_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_MESSAGE_LIST, SuiMessageList))
#define SUI_IS_MESSAGE_LIST(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_MESSAGE_LIST))

typedef struct _SuiMessageList SuiMessageList;
typedef struct _SuiMessageListClass SuiMessageListClass;

GType sui_message_list_get_type(void);
SuiMessageList *sui_message_list_new(void);

void sui_message_list_add_message(SuiMessageList *self, SuiMessage *msg, GtkAlign halign);

void sui_message_list_scroll_up(SuiMessageList *self, double step);
void sui_message_list_scroll_down(SuiMessageList *self, double step);

#endif /* __SUI_MESSAGE_LIST_H */
