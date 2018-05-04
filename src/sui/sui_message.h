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

#include "sui_notification.h"

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
    GtkLabel *message_box;
    GtkLabel *message_label;

    /* SuiMessage's style is varies with whether it has previous and next message.
     *
     * If a SuiMessage has not previous message, that it is a head of a serial
     * of composed message. And it has a style class named "sui-message-head".
     * If a SuiMessage has not next message, that it is a tail of a serial of
     * composed message. And it has a style class named "sui-message-tail".
     * A serial of composed message have the same widget width and same x
     * coordinate.  The ``min_x`` and ``max_width`` are used to do this stuff.
     *
     * Subclass should hide/show some widget in SuiMessageClass's
     * compose_prev/compose_next hander.
     *
     * NOTE: The prev and next pointer should only pointer to the same type of
     * intance with self.
     */
    SuiMessage *prev;
    SuiMessage *next;
    int min_x;
    int max_width;
};

struct _SuiMessageClass {
    GtkBoxClass parent_class;

    // Update the view of SuiMessage according self->ctx
    void (*update) (SuiMessage *self);
    // Compose self to previous message
    void (*compose_prev) (SuiMessage *self, SuiMessage *prev);
    // Compose self to next message
    void (*compose_next) (SuiMessage *self, SuiMessage *next);
    // New a SuiNotification for self
    SuiNotification* (*new_notification) (SuiMessage *self);
};

GType sui_message_get_type(void);

void sui_message_update(SuiMessage *self);
void sui_message_compose_prev(SuiMessage *self, SuiMessage *prev);
void sui_message_compose_next(SuiMessage *self, SuiMessage *next);
SuiNotification* sui_message_new_notification(SuiMessage *self);

void* sui_message_get_ctx(SuiMessage *self);
void sui_message_set_buffer(SuiMessage *self, SuiBuffer *buf);
SuiBuffer* sui_message_get_buffer(SuiMessage *self);
SuiMessage* sui_message_get_prev(SuiMessage *self);
SuiMessage* sui_message_get_next(SuiMessage *self);

void sui_message_label_on_popup(GtkLabel *label, GtkMenu *menu, gpointer user_data);

#endif /* __SUI_MESSAGE_H */
