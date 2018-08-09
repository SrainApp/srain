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

/**
 * @file sui_send_message.c
 * @brief Subclass of SuiMessage, used for displaying message sent by youself
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2018-04-06
 */

#include <gtk/gtk.h>

#include "sui_common.h"
#include "sui_send_message.h"

static void sui_send_message_update(SuiMessage *msg);
static void sui_send_message_compose_prev(SuiMessage *_self, SuiMessage *_prev);
static void sui_send_message_compose_next(SuiMessage *_self, SuiMessage *_next);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiSendMessage, sui_send_message, SUI_TYPE_MESSAGE);

static void sui_send_message_init(SuiSendMessage *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(SUI_MESSAGE(self)->message_label, "activate-link",
            G_CALLBACK(sui_common_activate_gtk_label_link), self);
    g_signal_connect(SUI_MESSAGE(self)->message_label, "populate-popup",
            G_CALLBACK(sui_message_label_on_popup), self);
}

static void sui_send_message_class_init(SuiSendMessageClass *class){
    GtkWidgetClass *widget_class;
    SuiMessageClass *message_class;

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/send_message.glade");
    gtk_widget_class_bind_template_child(widget_class, SuiMessage, content_box);
    gtk_widget_class_bind_template_child(widget_class, SuiMessage, message_label);
    gtk_widget_class_bind_template_child(widget_class, SuiSendMessage, time_label);

    message_class = SUI_MESSAGE_CLASS(class);
    message_class->update = sui_send_message_update;
    message_class->compose_prev = sui_send_message_compose_prev;
    message_class->compose_next = sui_send_message_compose_next;
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiSendMessage* sui_send_message_new(void *ctx){
    return g_object_new(SUI_TYPE_SEND_MESSAGE,
            "context", ctx,
            NULL);
}

static void sui_send_message_update(SuiMessage *msg){
    char *time;
    char *full_time;
    SrnMessage *ctx;
    SuiSendMessage *self;

    ctx = sui_message_get_ctx(msg);
    g_return_if_fail(ctx);
    self = SUI_SEND_MESSAGE(msg);

    time = g_date_time_format(ctx->time, "%R");
    full_time = g_date_time_format(ctx->time, "%c");
    g_return_if_fail(time);
    g_return_if_fail(full_time);

    gtk_label_set_text(self->time_label, time);
    gtk_widget_set_tooltip_text(GTK_WIDGET(self->time_label), full_time);

    g_free(full_time);
    g_free(time);

    SUI_MESSAGE_CLASS(sui_send_message_parent_class)->update(msg);
}

static void sui_send_message_compose_prev(SuiMessage *_self, SuiMessage *_prev){
    SUI_MESSAGE_CLASS(sui_send_message_parent_class)->compose_prev(_self, _prev);
}

static void sui_send_message_compose_next(SuiMessage *_self, SuiMessage *_next){
    SuiSendMessage *self;

    self = SUI_SEND_MESSAGE(_self);

    gtk_widget_hide(GTK_WIDGET(self->time_label));

    SUI_MESSAGE_CLASS(sui_send_message_parent_class)->compose_next(_self, _next);
}
