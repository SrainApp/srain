
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
 * @file srain_private_buffer.c
 * @brief SrainPrivateBuffer derived class which represents a private chatting
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-12-18
 */

#include <gtk/gtk.h>
#include "sui_event_hdr.h"
#include "srain_buffer.h"
#include "srain_chat_buffer.h"
#include "srain_private_buffer.h"

#include "log.h"

struct _SrainPrivateBuffer {
    SrainChatBuffer parent;

    GtkMenuItem *close_menu_item;
};

struct _SrainPrivateBufferClass {
    SrainChatBufferClass parent_class;
};

static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SrainPrivateBuffer, srain_private_buffer, SRAIN_TYPE_CHAT_BUFFER);

static void srain_private_buffer_init(SrainPrivateBuffer *self){
    GtkBuilder *builder;

    /* Init menus */
    builder = gtk_builder_new_from_resource("/org/gtk/srain/buffer_menu.glade");
    self->close_menu_item =
        (GtkMenuItem *)gtk_builder_get_object(builder, "close_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(srain_buffer_get_menu(SRAIN_BUFFER(self))),
            GTK_WIDGET(self->close_menu_item));
    g_object_unref(builder);

    g_signal_connect(self->close_menu_item, "activate",
            G_CALLBACK(close_menu_item_on_activate), self);
}

static void srain_private_buffer_finalize(GObject *object){
    G_OBJECT_CLASS(srain_private_buffer_parent_class)->finalize(object);
}

static void srain_private_buffer_class_init(SrainPrivateBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->finalize = srain_private_buffer_finalize;
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrainPrivateBuffer* srain_private_buffer_new(SuiSession *sui,
        SrainServerBuffer *srv, const char *nick){
    SrainPrivateBuffer *self;

    self = g_object_new(SRAIN_TYPE_CHAT_BUFFER,
            "session", sui,
            "server", srv,
            "name",   nick,
            NULL);

    return self;
}


/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SrainPrivateBuffer *self;

    self = user_data;

    sui_event_hdr(srain_buffer_get_session(SRAIN_BUFFER(self)),
                SUI_EVENT_UNQUERY, NULL);
}
