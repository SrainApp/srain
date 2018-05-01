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

#ifndef __SRAIN_MSG_LIST_H
#define __SRAIN_MSG_LIST_H

#include <gtk/gtk.h>
#include "sui_message.h"

#define SRAIN_TYPE_MSG_LIST (srain_msg_list_get_type())
#define SRAIN_MSG_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_MSG_LIST, SrainMsgList))
#define SRAIN_IS_MSG_LIST(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_MSG_LIST))

typedef struct _SrainMsgList SrainMsgList;
typedef struct _SrainMsgListClass SrainMsgListClass;

GType srain_msg_list_get_type(void);
SrainMsgList *srain_msg_list_new(void);

void srain_msg_list_add_message(SrainMsgList *list, SuiMessage *smsg, GtkAlign halign);
void srain_msg_list_highlight_message(SuiMessage *smsg);

void srain_msg_list_scroll_up(SrainMsgList *list, double step);
void srain_msg_list_scroll_down(SrainMsgList *list, double step);

#endif /* __SRAIN_MSG_LIST_H */
