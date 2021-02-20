
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
 * @file sui_dialog_buffer.c
 * @brief SuiDialogBuffer derived class which represents a private chatting
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-12-18
 */

#include <gtk/gtk.h>

#include "sui_event_hdr.h"
#include "sui_chat_buffer.h"
#include "sui_dialog_buffer.h"

#include "log.h"

struct _SuiDialogBuffer {
    SuiChatBuffer parent;

    // GtkMenuItem *close_menu_item;
};

struct _SuiDialogBufferClass {
    SuiChatBufferClass parent_class;
};

// static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiDialogBuffer, sui_dialog_buffer, SUI_TYPE_CHAT_BUFFER);

static void sui_dialog_buffer_init(SuiDialogBuffer *self){
    GtkBuilder *builder;

    /* Init menus */
    // builder = gtk_builder_new_from_resource("/im/srain/Srain/buffer_menu.glade");
    // self->close_menu_item =
    //     (GtkMenuItem *)gtk_builder_get_object(builder, "close_menu_item");
    // gtk_menu_shell_append(
    //         GTK_MENU_SHELL(sui_buffer_get_menu(SUI_BUFFER(self))),
    //         GTK_WIDGET(self->close_menu_item));
    // g_object_unref(builder);

    // g_signal_connect(self->close_menu_item, "activate",
            // G_CALLBACK(close_menu_item_on_activate), self);
}

static void sui_dialog_buffer_finalize(GObject *object){
    G_OBJECT_CLASS(sui_dialog_buffer_parent_class)->finalize(object);
}

static void sui_dialog_buffer_class_init(SuiDialogBufferClass *class){
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS(class);

    object_class->finalize = sui_dialog_buffer_finalize;

    widget_class = GTK_WIDGET_CLASS(class);

    gtk_widget_class_set_template_from_resource(
            widget_class, "/im/srain/Srain/buffer.glade");

    // gtk_widget_class_bind_template_child_full(widget_class,
    //         "close_menu_item",
    //         FALSE,
    //         G_STRUCT_OFFSET(SuiDialogBuffer, close_menu_item));
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiDialogBuffer* sui_dialog_buffer_new(void *ctx, SuiBufferEvents *events,
        SuiBufferConfig *cfg){
    SuiDialogBuffer *self;

    self = g_object_new(SUI_TYPE_DIALOG_BUFFER,
            "context", ctx,
            "events", events,
            "config", cfg,
            NULL);

    return self;
}


/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiDialogBuffer *self;

    self = user_data;

    sui_buffer_event_hdr(SUI_BUFFER(self), SUI_EVENT_UNQUERY, NULL);
}
