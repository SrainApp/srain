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
 * @file sui_chat_buffer.c
 * @brief
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-12-17
 */

#include <gtk/gtk.h>

#include "sui_buffer.h"
#include "sui_chat_buffer.h"

#include "log.h"

static void user_list_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/
enum
{
    // 0 for PROP_NOME
    PROP_SERVER_BUFFER = 1,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiChatBuffer, sui_chat_buffer, SUI_TYPE_BUFFER);

static void sui_chat_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
    SuiChatBuffer *self = SUI_CHAT_BUFFER(object);

    switch (property_id){
        case PROP_SERVER_BUFFER:
            self->server_buffer = SUI_SERVER_BUFFER(g_value_get_pointer(value));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void sui_chat_buffer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
    SuiChatBuffer *self = SUI_CHAT_BUFFER(object);

    switch (property_id){
        case PROP_SERVER_BUFFER:
            g_value_set_pointer(value,
                    sui_chat_buffer_get_server_buffer(self));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void sui_chat_buffer_constructed(GObject *object){
    SrnChat *ctx;
    SuiServerBuffer *srv;
    SuiChatBuffer *self;

    g_object_get(object, "context", &ctx, NULL);
    g_return_if_fail(ctx);
    g_return_if_fail(SUI_IS_SERVER_BUFFER(ctx->srv->chat->ui));

    srv = SUI_SERVER_BUFFER(ctx->srv->chat->ui);
    self = SUI_CHAT_BUFFER(object);

    // Overwrite properties
    g_object_set(object, "server-buffer", srv, NULL);
    // Add self to server buffer list
    sui_server_buffer_add_buffer(srv, SUI_BUFFER(self));

    G_OBJECT_CLASS(sui_chat_buffer_parent_class)->constructed(object);
}

static void sui_chat_buffer_init(SuiChatBuffer *self){
    GtkBuilder *builder;

    /* Init menus */
    builder = gtk_builder_new_from_resource("/im/srain/Srain/buffer_menu.glade");
    self->user_list_menu_item =
        (GtkCheckMenuItem *)gtk_builder_get_object(builder, "user_list_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(sui_buffer_get_menu(SUI_BUFFER(self))),
            GTK_WIDGET(self->user_list_menu_item));
    g_object_unref(builder);
    
    /* Init user list*/
    self->user_list = srain_user_list_new();
    // Hide user_list for avoiding warning
    gtk_widget_hide(GTK_WIDGET(self->user_list));
    gtk_container_add(GTK_CONTAINER(self->parent.user_list_revealer), // FIXME
            GTK_WIDGET(self->user_list));

    gtk_widget_show(GTK_WIDGET(self->user_list_menu_item));

    g_signal_connect(self->user_list_menu_item, "toggled",
            G_CALLBACK(user_list_menu_item_on_toggled), self);
}

static void sui_chat_buffer_finalize(GObject *object){
    SuiChatBuffer *self;

    self = SUI_CHAT_BUFFER(object);
    sui_server_buffer_rm_buffer(self->server_buffer, SUI_BUFFER(self));

    G_OBJECT_CLASS(sui_chat_buffer_parent_class)->finalize(object);
}

static void sui_chat_buffer_class_init(SuiChatBufferClass *class){
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS(class);

    /* Overwrite callbacks */
    object_class->constructed = sui_chat_buffer_constructed;
    object_class->finalize = sui_chat_buffer_finalize;
    object_class->set_property = sui_chat_buffer_set_property;
    object_class->get_property = sui_chat_buffer_get_property;

    /* Install properties */
    obj_properties[PROP_SERVER_BUFFER] =
        g_param_spec_pointer("server-buffer", // TODO: use spec gobject
                "Server Buffer",
                "Server buffer of buffer",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiServerBuffer* sui_chat_buffer_get_server_buffer(SuiChatBuffer *self){
    return self->server_buffer;
}

SrainUserList* sui_chat_buffer_get_user_list(SuiChatBuffer *self){
    return self->user_list;
}

void sui_chat_buffer_show_user_list(SuiChatBuffer *self, bool isshow){
    gtk_check_menu_item_set_active(self->user_list_menu_item, isshow);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void user_list_menu_item_on_toggled(GtkWidget* widget, gpointer user_data){
    bool active;
    SuiChatBuffer *self = SUI_CHAT_BUFFER(user_data);
    GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM(widget);

    active = gtk_check_menu_item_get_active(item);
    gtk_revealer_set_reveal_child(self->parent.user_list_revealer, active);
    if (active){
        gtk_widget_show(GTK_WIDGET(self->user_list));
    } else {
        gtk_widget_hide(GTK_WIDGET(self->user_list));
    }
}
