/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __SUI_RECV_MESSAGE_H
#define __SUI_RECV_MESSAGE_H

#include <gtk/gtk.h>

#include "sui_message.h"

#define SUI_TYPE_RECV_MESSAGE (sui_recv_message_get_type())
#define SUI_RECV_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_RECV_MESSAGE, SuiRecvMessage))
#define SUI_IS_RECV_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_RECV_MESSAGE))

typedef struct _SuiRecvMessage SuiRecvMessage;
typedef struct _SuiRecvMessageClass SuiRecvMessageClass;

struct _SuiRecvMessage {
    SuiMessage parent;

    GtkEventBox *nick_event_box;
    GtkLabel *nick_label;
    GtkLabel *identify_label;
    GtkLabel *time_label;
};

struct _SuiRecvMessageClass {
    SuiMessageClass parent_class;
};

GType sui_recv_message_get_type(void);
SuiRecvMessage *sui_recv_message_new(void *ctx);

#endif /* __SUI_RECV_MESSAGE_H */
