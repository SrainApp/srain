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
 * @file srain_server_buffer.c
 * @brief SrainBuffer derived class which represents a session to server
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-11-26
 */

#include <gtk/gtk.h>
#include "sui_event_hdr.h"
#include "srain_buffer.h"
#include "srain_server_buffer.h"

#include "log.h"
#include "i18n.h"

struct _SrainServerBuffer {
    SrainBuffer parent;

    GtkMenuItem *disconn_menu_item;
    GtkMenuItem *quit_menu_item;

    GSList *buffer_list;
    GtkListStore *chan_list_store;
};

struct _SrainServerBufferClass {
    SrainBufferClass parent_class;
};

static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SrainServerBuffer, srain_server_buffer, SRAIN_TYPE_BUFFER);

static void srain_server_buffer_init(SrainServerBuffer *self){
    GtkBuilder *builder;

    self->buffer_list = NULL;

    /* Init menus */
    builder = gtk_builder_new_from_resource("/org/gtk/srain/buffer_menu.glade");
    self->disconn_menu_item =
        (GtkMenuItem *)gtk_builder_get_object(builder, "disconn_menu_item");
    self->quit_menu_item =
        (GtkMenuItem *)gtk_builder_get_object(builder, "quit_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(srain_buffer_get_menu(SRAIN_BUFFER(self))),
            GTK_WIDGET(self->disconn_menu_item));
    gtk_menu_shell_append(
            GTK_MENU_SHELL(srain_buffer_get_menu(SRAIN_BUFFER(self))),
            GTK_WIDGET(self->quit_menu_item));
    g_object_unref(builder);

    g_signal_connect(self->disconn_menu_item, "activate",
            G_CALLBACK(disconn_menu_item_on_activate),self);
    g_signal_connect(self->quit_menu_item, "activate",
            G_CALLBACK(quit_menu_item_on_activate),self);
}

static void srain_server_buffer_finalize(GObject *object){
    SrainServerBuffer *self = SRAIN_SERVER_BUFFER(object);

    g_slist_free(self->buffer_list);

    G_OBJECT_CLASS(srain_server_buffer_parent_class)->finalize(object);
}

static void srain_server_buffer_class_init(SrainServerBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->finalize = srain_server_buffer_finalize;
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrainServerBuffer* srain_server_buffer_new(SuiSession *sui, const char *server){
    SrainServerBuffer *self;

    self = g_object_new(SRAIN_TYPE_SERVER_BUFFER,
            "session", sui,
            "name", _("Server"),
            "remark", server,
            NULL);

    return self;
}

void srain_server_buffer_add_buffer(SrainServerBuffer *self, SrainBuffer *buffer){
    GSList *lst;

    g_return_if_fail(!SRAIN_IS_SERVER_BUFFER(buffer));

    lst = self->buffer_list;
    while (lst){
        g_return_if_fail(lst->data != buffer);
        lst = g_slist_next(lst);
    }

    self->buffer_list = g_slist_append(self->buffer_list, buffer);
}

void srain_server_buffer_rm_buffer(SrainServerBuffer *self, SrainBuffer *buffer){
    GSList *lst;

    g_return_if_fail(!SRAIN_IS_SERVER_BUFFER(buffer));

    lst = self->buffer_list;
    while (lst){
        if (lst->data == buffer){
            self->buffer_list = g_slist_delete_link(self->buffer_list, lst);
            return;
        }
        lst = g_slist_next(lst);
    }

    g_return_if_reached();
}

GSList* srain_server_buffer_get_buffer_list(SrainServerBuffer *self){
    return self->buffer_list;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SrainServerBuffer *self;

    self = user_data;

    sui_event_hdr(srain_buffer_get_session(SRAIN_BUFFER(self)),
                SUI_EVENT_DISCONNECT, NULL);
}

static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SrainServerBuffer *self;

    self = user_data;

    // TODO: QUIT event
    sui_event_hdr(srain_buffer_get_session(SRAIN_BUFFER(self)),
                SUI_EVENT_DISCONNECT, NULL);
}
