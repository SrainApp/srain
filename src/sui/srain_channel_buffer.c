/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @file srain_channel_buffer.c
 * @brief SrainChannelBuffer derived class which represents a channel chatting
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-12-18
 */

#include <gtk/gtk.h>
#include "sui_event_hdr.h"
#include "srain_buffer.h"
#include "srain_chat_buffer.h"
#include "srain_channel_buffer.h"

#include "log.h"

struct _SrainChannelBuffer {
    SrainChatBuffer parent;

    GtkMenuItem *leave_menu_item;
};

struct _SrainChannelBufferClass {
    SrainChatBufferClass parent_class;
};

static void leave_menu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SrainChannelBuffer, srain_channel_buffer, SRAIN_TYPE_CHAT_BUFFER);

static void srain_channel_buffer_init(SrainChannelBuffer *self){
    GtkBuilder *builder;

    /* Init menus */
    builder = gtk_builder_new_from_resource("/org/gtk/srain/buffer_menu.glade");
    self->leave_menu_item =
        (GtkMenuItem *)gtk_builder_get_object(builder, "leave_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(srain_buffer_get_menu(SRAIN_BUFFER(self))),
            GTK_WIDGET(self->leave_menu_item));
    g_object_unref(builder);

    g_signal_connect(self->leave_menu_item, "activate",
            G_CALLBACK(leave_menu_item_on_activate), self);
}

static void srain_channel_buffer_finalize(GObject *object){
    G_OBJECT_CLASS(srain_channel_buffer_parent_class)->finalize(object);
}

static void srain_channel_buffer_class_init(SrainChannelBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->finalize = srain_channel_buffer_finalize;
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrainChannelBuffer* srain_channel_buffer_new(SuiSession *sui,
        SrainServerBuffer *srv, const char *chan){
    SrainChannelBuffer *self;

    self = g_object_new(SRAIN_TYPE_CHAT_BUFFER,
            "session", sui,
            "server", srv,
            "name",   chan,
            NULL);

    return self;
}


/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void leave_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SrainChannelBuffer *self;

    self = user_data;

    sui_event_hdr(srain_buffer_get_session(SRAIN_BUFFER(self)),
                SUI_EVENT_PART, NULL);
}
