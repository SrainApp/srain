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
 * @file sui_recv_message.c
 * @brief Subclass of SuiMessage, used for displaying received message from
 * other users
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2018-04-06
 */

#include <gtk/gtk.h>

#include "sui_common.h"
#include "nick_menu.h"
#include "sui_recv_message.h"
#include "gtk_compat.h"

#include "i18n.h"

static void sui_recv_message_update(SuiMessage *msg);
static void sui_recv_message_compose_prev(SuiMessage *_self, SuiMessage *_prev);
static void sui_recv_message_compose_next(SuiMessage *_self, SuiMessage *_next);

#if GTK_MAJOR_VERSION >= 4
static void sender_event_box_on_click(GtkGestureClick *gesture, int n_press,
        double x, double y, gpointer user_data);
#else
static void sender_event_box_on_button_press(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data);
static void sender_event_box_on_button_release(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data);
#endif

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiRecvMessage, sui_recv_message, SUI_TYPE_MESSAGE);

static void sui_recv_message_init(SuiRecvMessage *self){
#if GTK_MAJOR_VERSION >= 4
    GtkGesture *click;
#endif

    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(SUI_MESSAGE(self)->message_label, "activate-link",
            G_CALLBACK(sui_common_activate_gtk_label_link), self);
    sui_message_label_connect_popup(SUI_MESSAGE(self)->message_label, self);
#if GTK_MAJOR_VERSION >= 4
    click = gtk_gesture_click_new();
    gtk_widget_add_controller(self->sender_event_box,
            GTK_EVENT_CONTROLLER(click));
    g_signal_connect(click, "released",
            G_CALLBACK(sender_event_box_on_click), self);
#else
    g_signal_connect(self->sender_event_box, "button-press-event",
            G_CALLBACK(sender_event_box_on_button_press), self);
    g_signal_connect(self->sender_event_box, "button-release-event",
            G_CALLBACK(sender_event_box_on_button_release), self);
#endif

}

static void sui_recv_message_class_init(SuiRecvMessageClass *class){
    GtkWidgetClass *widget_class;
    SuiMessageClass *message_class;

    widget_class = GTK_WIDGET_CLASS(class);
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/recv_message.ui");
#else
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/recv_message.glade");
#endif
    gtk_widget_class_bind_template_child(widget_class, SuiMessage, content_box);
    gtk_widget_class_bind_template_child(widget_class, SuiMessage, message_label);
    gtk_widget_class_bind_template_child(widget_class, SuiRecvMessage, time_label);
    gtk_widget_class_bind_template_child(widget_class, SuiRecvMessage, sender_box);
    gtk_widget_class_bind_template_child(widget_class, SuiRecvMessage, sender_event_box);
    gtk_widget_class_bind_template_child(widget_class, SuiRecvMessage, sender_label);
    gtk_widget_class_bind_template_child(widget_class, SuiRecvMessage, remark_label);

    message_class = SUI_MESSAGE_CLASS(class);
    message_class->update = sui_recv_message_update;
    message_class->compose_prev = sui_recv_message_compose_prev;
    message_class->compose_next = sui_recv_message_compose_next;
}

static void sui_recv_message_update(SuiMessage *_self){
    const char *time;
    const char *full_time;
    SrnMessage *ctx;
    SuiRecvMessage *self;

    ctx = sui_message_get_ctx(_self);
    g_return_if_fail(ctx);
    self = SUI_RECV_MESSAGE(_self);

    gtk_label_set_markup(self->sender_label, ctx->rendered_sender);
    if (ctx->rendered_remark) {
        gtk_label_set_text(self->remark_label, ctx->rendered_remark);
    }

    time =  sui_message_get_time(_self);
    full_time = sui_message_get_full_time(_self);
    g_return_if_fail(time);
    g_return_if_fail(full_time);

    gtk_label_set_text(self->time_label, time);
    gtk_widget_set_tooltip_text(GTK_WIDGET(self->time_label), full_time);

    SUI_MESSAGE_CLASS(sui_recv_message_parent_class)->update(_self);
}

static void sui_recv_message_compose_prev(SuiMessage *_self, SuiMessage *_prev){
    SuiRecvMessage *self;

    self = SUI_RECV_MESSAGE(_self);

    srn_gtk_widget_hide(GTK_WIDGET(self->sender_box));

    SUI_MESSAGE_CLASS(sui_recv_message_parent_class)->compose_prev(_self, _prev);
}

static void sui_recv_message_compose_next(SuiMessage *_self, SuiMessage *_next){
    SuiRecvMessage *self;

    self = SUI_RECV_MESSAGE(_self);

    srn_gtk_widget_hide(GTK_WIDGET(self->time_label));

    SUI_MESSAGE_CLASS(sui_recv_message_parent_class)->compose_next(_self, _next);
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiRecvMessage *sui_recv_message_new(void *ctx){
    return g_object_new(SUI_TYPE_RECV_MESSAGE,
            "context", ctx,
            NULL);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

#if GTK_MAJOR_VERSION >= 4
static void sender_event_box_on_click(GtkGestureClick *gesture, int n_press,
        double x, double y, gpointer user_data){
    char *insert_text;
    guint button;
    SuiBuffer *buf;
    SuiRecvMessage *self;
    GtkWidget *widget;

    button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    self = SUI_RECV_MESSAGE(user_data);
    widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

    if (button == GDK_BUTTON_SECONDARY){
        nick_menu_popup(widget, gtk_label_get_text(self->sender_label));
        return;
    }
    if (button != GDK_BUTTON_PRIMARY){
        return;
    }

    insert_text = g_strdup_printf("%s: ", gtk_label_get_text(self->sender_label));
    buf = sui_message_get_buffer(SUI_MESSAGE(self));
    sui_buffer_insert_text(buf, insert_text, -1, 0);
    g_free(insert_text);
}
#else
static void sender_event_box_on_button_press(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    SuiRecvMessage *self;

    if (event->button != 3){ // Left mouse button
        return;
    }

    self = SUI_RECV_MESSAGE(user_data);
    nick_menu_popup(widget, gtk_label_get_text(self->sender_label));
}

static void sender_event_box_on_button_release(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    char *insert_text;
    SuiBuffer *buf;
    SuiRecvMessage *self;

    if (event->button != 1){ // Left mouse button
        return;
    }

    self = SUI_RECV_MESSAGE(user_data);
    insert_text = g_strdup_printf("%s: ", gtk_label_get_text(self->sender_label));
    buf = sui_message_get_buffer(SUI_MESSAGE(self));
    sui_buffer_insert_text(buf, insert_text, -1, 0);
    g_free(insert_text);
}
#endif
