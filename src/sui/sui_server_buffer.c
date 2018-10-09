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

/**
 * @file sui_server_buffer.c
 * @brief
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-11-26
 */

#include <gtk/gtk.h>

#include "sui_event_hdr.h"
#include "sui_buffer.h"
#include "sui_server_buffer.h"

#include "log.h"
#include "i18n.h"

struct _SuiServerBuffer {
    SuiBuffer parent;

    GList *buffer_list;

    GtkMenuItem *disconn_menu_item;
    GtkMenuItem *quit_menu_item;

    GtkListStore *chan_list_store;
    SuiJoinPanel *join_panel;
};

struct _SuiServerBufferClass {
    SuiBufferClass parent_class;
};

static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum {
    // 0 for PROP_NOME
    N_PROPERTIES = 1
};

// static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiServerBuffer, sui_server_buffer, SUI_TYPE_BUFFER);

static void sui_server_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  switch (property_id){
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_server_buffer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  switch (property_id){
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_server_buffer_init(SuiServerBuffer *self){
    GtkBuilder *builder;
    
    /* Init menus */
    builder = gtk_builder_new_from_resource("/im/srain/Srain/buffer_menu.glade");
    self->disconn_menu_item =
        (GtkMenuItem *)gtk_builder_get_object(builder, "disconn_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(sui_buffer_get_menu(SUI_BUFFER(self))),
            GTK_WIDGET(self->disconn_menu_item));
    self->quit_menu_item =
        (GtkMenuItem *)gtk_builder_get_object(builder, "quit_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(sui_buffer_get_menu(SUI_BUFFER(self))),
            GTK_WIDGET(self->quit_menu_item));
    g_object_unref(builder);

    self->buffer_list = NULL;

    /* Init channel list */
    self->chan_list_store = gtk_list_store_new(3,
            G_TYPE_STRING,
            G_TYPE_INT,
            G_TYPE_STRING);
    self->join_panel = g_object_ref(sui_join_panel_new());
    sui_join_panel_set_model(self->join_panel,
            GTK_TREE_MODEL(self->chan_list_store));

    g_signal_connect(self->disconn_menu_item, "activate",
            G_CALLBACK(disconn_menu_item_on_activate),self);
    g_signal_connect(self->quit_menu_item, "activate",
            G_CALLBACK(quit_menu_item_on_activate),self);
}

static void sui_server_buffer_finalize(GObject *object){
    SuiServerBuffer *self = SUI_SERVER_BUFFER(object);

    g_warn_if_fail(g_list_length(self->buffer_list) == 0);

    g_object_unref(self->chan_list_store);
    g_object_unref(self->join_panel);

    G_OBJECT_CLASS(sui_server_buffer_parent_class)->finalize(object);
}

static void sui_server_buffer_class_init(SuiServerBufferClass *class){
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS(class);

    object_class->finalize = sui_server_buffer_finalize;
    object_class->set_property = sui_server_buffer_set_property;
    object_class->get_property = sui_server_buffer_get_property;
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiServerBuffer* sui_server_buffer_new(void *ctx, SuiBufferEvents *events,
        SuiBufferConfig *cfg){
    SuiServerBuffer *self;

    self = g_object_new(SUI_TYPE_SERVER_BUFFER,
            "context", ctx,
            "events", events,
            "config", cfg,
            NULL);

    return self;
}

void sui_server_buffer_add_buffer(SuiServerBuffer *self, SuiBuffer *buf){
    GList *lst;

    g_return_if_fail(!SUI_IS_SERVER_BUFFER(buf));

    lst = self->buffer_list;
    while (lst){
        g_return_if_fail(lst->data != buf);
        lst = g_list_next(lst);
    }

    self->buffer_list = g_list_append(self->buffer_list, buf);
}

void sui_server_buffer_rm_buffer(SuiServerBuffer *self, SuiBuffer *buf){
    GList *lst;

    g_return_if_fail(!SUI_IS_SERVER_BUFFER(buf));

    lst = self->buffer_list;
    while (lst){
        if (lst->data == buf){
            self->buffer_list = g_list_delete_link(self->buffer_list, lst);
            return;
        }
        lst = g_list_next(lst);
    }

    g_return_if_reached();
}

GList* sui_server_buffer_get_buffer_list(SuiServerBuffer *self){
    return self->buffer_list;
}

SuiJoinPanel* sui_server_buffer_get_join_panel(SuiServerBuffer *self){
    return self->join_panel;
}

void sui_server_buffer_add_channel(SuiServerBuffer *self,
        const char *chan, int users, const char *topic){
    GtkTreeIter iter;
    GtkListStore *store;

    store = self->chan_list_store;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            CHANNEL_LIST_STORE_COL_CHANNEL, chan,
            CHANNEL_LIST_STORE_COL_USERS, users,
            CHANNEL_LIST_STORE_COL_TOPIC, topic,
            -1);
}

void sui_server_buffer_clear_channel(SuiServerBuffer *self){
    gtk_list_store_clear(self->chan_list_store);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiServerBuffer *self;

    self = user_data;

    sui_buffer_event_hdr(SUI_BUFFER(self), SUI_EVENT_DISCONNECT, NULL);
}

static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiServerBuffer *self;

    self = user_data;

    sui_buffer_event_hdr(SUI_BUFFER(self), SUI_EVENT_QUIT, NULL);
}
