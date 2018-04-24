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

#ifndef __SUI_MESSAGE_H
#define __SUI_MESSAGE_H

#include <gtk/gtk.h>

#include "core/core.h"
#include "sui/sui.h"

/*****************************************************************************
 * SuiMessage
 *****************************************************************************/

#define SUI_TYPE_MESSAGE (sui_message_get_type())
#define SUI_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_MESSAGE, SuiMessage))
#define SUI_IS_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_MESSAGE))
#define SUI_MESSAGE_CLASS(class) (G_TYPE_CHECK_CLASS_CAST((class), SUI_TYPE_MESSAGE, SuiMessageClass))
#define SUI_MESSAGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SUI_TYPE_MESSAGE, SuiMessageClass))

typedef struct _SuiMessage SuiMessage;
typedef struct _SuiMessageClass SuiMessageClass;

struct _SuiMessage {
    GtkBox parent;
    SrnMessage *ctx;
    SuiBuffer *buf;
    GtkLabel *message_label;
};

struct _SuiMessageClass {
    GtkBoxClass parent_class;

    void (*update) (SuiMessage *self);

    /* Padding to allow adding up to 12 new virtual functions without
     * breaking ABI */
    gpointer padding[12];
};

GType sui_message_get_type(void);

void sui_message_update(SuiMessage *self);

void* sui_message_get_ctx(SuiMessage *self);
void sui_message_set_buffer(SuiMessage *self, SuiBuffer *buf);
SuiBuffer* sui_message_get_buffer(SuiMessage *self);
void sui_message_label_on_popup(GtkLabel *label, GtkMenu *menu, gpointer user_data);

#endif /* __SUI_MESSAGE_H */
