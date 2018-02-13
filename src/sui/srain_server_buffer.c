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
 * @file srain_server_buffer.c
 * @brief SrainBuffer derived class which represents a session to server
 * @author Shengyu Zhang <i@silverrainz.me>
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

    bool adding_chan;

    GtkMenuItem *disconn_menu_item;
    GtkMenuItem *quit_menu_item;

    GSList *buffer_list;
    GtkListStore *chan_list_store;
    GtkTreeModelFilter *chan_tree_model_filter; // FilterTreeModel of chan_list_store
};

struct _SrainServerBufferClass {
    SrainBufferClass parent_class;
};

static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void srain_server_buffer_set_adding_channel(SrainServerBuffer *self, bool adding_chan);
static bool srain_server_buffer_get_adding_channel(SrainServerBuffer *self);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_ADDING_CHANNEL = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SrainServerBuffer, srain_server_buffer, SRAIN_TYPE_BUFFER);

static void srain_server_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SrainServerBuffer *self = SRAIN_SERVER_BUFFER(object);

  switch (property_id){
    case PROP_ADDING_CHANNEL:
      srain_server_buffer_set_adding_channel(self, g_value_get_boolean(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void srain_server_buffer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SrainServerBuffer *self = SRAIN_SERVER_BUFFER(object);

  switch (property_id){
    case PROP_ADDING_CHANNEL:
      g_value_set_boolean(value, srain_server_buffer_get_adding_channel(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void srain_server_buffer_init(SrainServerBuffer *self){
    GtkBuilder *builder;

    self->adding_chan = FALSE;
    self->buffer_list = NULL;

    /* Init data model */
    self->chan_list_store = gtk_list_store_new(3,
            G_TYPE_STRING,
            G_TYPE_INT,
            G_TYPE_STRING);
    self->chan_tree_model_filter = GTK_TREE_MODEL_FILTER(
            gtk_tree_model_filter_new(GTK_TREE_MODEL(self->chan_list_store), NULL));

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
    g_object_unref(self->chan_list_store);
    // FIXME

    G_OBJECT_CLASS(srain_server_buffer_parent_class)->finalize(object);
}

static void srain_server_buffer_class_init(SrainServerBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->finalize = srain_server_buffer_finalize;
    object_class->set_property = srain_server_buffer_set_property;
    object_class->get_property = srain_server_buffer_get_property;

    obj_properties[PROP_ADDING_CHANNEL] =
        g_param_spec_boolean("adding-channel",
                "Adding channel",
                "A flag represents that whether this buffer is adding channel.",
                FALSE,
                G_PARAM_READWRITE);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrainServerBuffer* srain_server_buffer_new(SuiSession *sui, const char *server){
    SrainServerBuffer *self;

    self = g_object_new(SRAIN_TYPE_SERVER_BUFFER,
            "session", sui,
            "name", META_SERVER,
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

GtkTreeModelFilter* srain_server_buffer_get_channel_model(
        SrainServerBuffer *self){
    return self->chan_tree_model_filter;
}

void srain_server_buffer_start_add_channel(SrainServerBuffer *self){
    gtk_list_store_clear(self->chan_list_store);
    srain_server_buffer_set_adding_channel(self, TRUE);
}

void srain_server_buffer_add_channel(SrainServerBuffer *self,
        const char *chan, int users, const char *topic){
    GtkTreeIter iter;
    GtkListStore *store;

    g_return_if_fail(self->adding_chan);

    store = self->chan_list_store;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            CHANNEL_LIST_STORE_COL_CHANNEL, chan,
            CHANNEL_LIST_STORE_COL_USERS, users,
            CHANNEL_LIST_STORE_COL_TOPIC, topic,
            -1);
}

void srain_server_buffer_end_add_channel(SrainServerBuffer *self){
    srain_server_buffer_set_adding_channel(self, FALSE);
}

bool srain_server_buffer_is_adding_channel(SrainServerBuffer *self){
    return srain_server_buffer_get_adding_channel(self);
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

    sui_event_hdr(srain_buffer_get_session(SRAIN_BUFFER(self)),
            SUI_EVENT_QUIT, NULL);
}

static void srain_server_buffer_set_adding_channel(SrainServerBuffer *self,
        bool adding_chan){
    g_object_freeze_notify(G_OBJECT(self));

    self->adding_chan = adding_chan;

    g_object_notify_by_pspec(G_OBJECT(self),
            obj_properties[PROP_ADDING_CHANNEL]);

    g_object_thaw_notify(G_OBJECT(self));
}

static bool srain_server_buffer_get_adding_channel(SrainServerBuffer *self){
    return self->adding_chan;
}
