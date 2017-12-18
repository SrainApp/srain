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
 * @file srain_chat_buffer.c
 * @brief SrainBuffer derived class which represents a general chatting
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-12-17
 */

#include <gtk/gtk.h>
#include "srain_buffer.h"
#include "srain_chat_buffer.h"

#include "log.h"

static void user_list_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/
enum
{
  // 0 for PROP_NOME
  PROP_SERVER = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SrainChatBuffer, srain_chat_buffer, SRAIN_TYPE_BUFFER);

static void srain_chat_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SrainChatBuffer *self = SRAIN_CHAT_BUFFER(object);

  switch (property_id){
    case PROP_SERVER:
      self->server_buffer = SRAIN_SERVER_BUFFER(g_value_get_pointer(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void srain_chat_buffer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SrainChatBuffer *self = SRAIN_CHAT_BUFFER(object);

  switch (property_id){
    case PROP_SERVER:
      g_value_set_pointer(value,
              srain_chat_buffer_get_server_buffer(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static GObject *srain_chat_buffer_constructor(GType type,
        guint n_construct_properties, GObjectConstructParam *construct_params){
    GObject *object;
    SrainBuffer *buffer;

    object = G_OBJECT_CLASS(srain_chat_buffer_parent_class)->
        constructor(type, n_construct_properties, construct_params);

    g_object_get(object, "server", &buffer, NULL);
    g_object_set(object, "remark", srain_buffer_get_remark(buffer), NULL);

    return object;
}

static void srain_chat_buffer_init(SrainChatBuffer *self){
    GtkBuilder *builder;

    /* Init menus */
    builder = gtk_builder_new_from_resource("/org/gtk/srain/buffer_menu.glade");
    self->user_list_menu_item =
        (GtkCheckMenuItem *)gtk_builder_get_object(builder, "user_list_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(srain_buffer_get_menu(SRAIN_BUFFER(self))),
            GTK_WIDGET(self->user_list_menu_item));
    g_object_unref(builder);

    /* Init user list*/
    self->user_list = srain_user_list_new();

    // Hide user_list for avoiding warning
    gtk_widget_hide(GTK_WIDGET(self->user_list));
    gtk_container_add(GTK_CONTAINER(self->parent.user_list_revealer), // FIXME
            GTK_WIDGET(self->user_list));

    g_signal_connect(self->user_list_menu_item, "toggled",
            G_CALLBACK(user_list_menu_item_on_toggled), self);
}

static void srain_chat_buffer_finalize(GObject *object){
    G_OBJECT_CLASS(srain_chat_buffer_parent_class)->finalize(object);
}

static void srain_chat_buffer_class_init(SrainChatBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    /* Overwrite callbacks */
    object_class->constructor = srain_chat_buffer_constructor;
    object_class->finalize = srain_chat_buffer_finalize;
    object_class->set_property = srain_chat_buffer_set_property;
    object_class->get_property = srain_chat_buffer_get_property;

    /* Install properties */
    obj_properties[PROP_SERVER] =
        g_param_spec_pointer("server",
                "Server",
                "Server buffer of buffer",
                G_PARAM_READWRITE);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrainServerBuffer* srain_chat_buffer_get_server_buffer(SrainChatBuffer *self){
    return self->server_buffer;
}

SrainUserList* srain_chat_buffer_get_user_list(SrainChatBuffer *self){
    return self->user_list;
}

void srain_chat_buffer_show_user_list(SrainChatBuffer *self, bool isshow){
    gtk_check_menu_item_set_active(self->user_list_menu_item, isshow);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void user_list_menu_item_on_toggled(GtkWidget* widget, gpointer user_data){
    bool active;
    SrainChatBuffer *self = SRAIN_CHAT_BUFFER(user_data);
    GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM(widget);

    active = gtk_check_menu_item_get_active(item);
    gtk_revealer_set_reveal_child(self->parent.user_list_revealer, active); // FIXME
    if (active){
        gtk_widget_show(GTK_WIDGET(self->user_list));
    } else {
        gtk_widget_hide(GTK_WIDGET(self->user_list));
    }
}
