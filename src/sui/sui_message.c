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
 * @file sui_message.c
 * @brief Base class of all messages
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2018-04-06
 */

#include <time.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "nick_menu.h"
#include "sui_window.h"
#include "sui_buffer.h"
#include "sui_message.h"

#include "log.h"
#include "i18n.h"
#include "meta.h"
#include "utils.h"

static void sui_message_real_update(SuiMessage *self);

static void sui_message_set_ctx(SuiMessage *self, void *ctx);

static char* label_get_selection(GtkLabel *label);
static void copy_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void froward_submenu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum {
  // 0 for PROP_NOME
  PROP_CTX = 1,
  PROP_BUFFER,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiMessage, sui_message, GTK_TYPE_BOX);

static void sui_message_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiMessage *self = SUI_MESSAGE(object);

  switch (property_id){
      case PROP_CTX:
          sui_message_set_ctx(self, g_value_get_pointer(value));
          break;
      case PROP_BUFFER:
          sui_message_set_buffer(self, g_value_get_pointer(value));
          break;
      default:
          /* We don't have any other property... */
          G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
          break;
  }
}

static void sui_message_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiMessage *self = SUI_MESSAGE(object);

  switch (property_id){
    case PROP_CTX:
      g_value_set_pointer(value, sui_message_get_ctx(self));
      break;
    case PROP_BUFFER:
      g_value_set_pointer(value, sui_message_get_buffer(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_message_init(SuiMessage *self){
}

static void sui_message_constructed(GObject *object){
    G_OBJECT_CLASS(sui_message_parent_class)->constructed(object);
}

static void sui_message_finalize(GObject *object){
    G_OBJECT_CLASS(sui_message_parent_class)->finalize(object);
}

static void sui_message_class_init(SuiMessageClass *class){
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = sui_message_constructed;
    object_class->finalize = sui_message_finalize;
    object_class->set_property = sui_message_set_property;
    object_class->get_property = sui_message_get_property;

    /* Install properties */
    obj_properties[PROP_CTX] =
        g_param_spec_pointer("context",
                "Context",
                "Context of message.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_BUFFER] =
        g_param_spec_pointer("buffer",
                "Buffer",
                "Buffer of message.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);

    class->update = sui_message_real_update;
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

void* sui_message_get_ctx(SuiMessage *self){
    return self->ctx;
}

void sui_message_set_buffer(SuiMessage *self, SuiBuffer *buf){
    self->buf = buf;
}

SuiBuffer* sui_message_get_buffer(SuiMessage *self){
    return self->buf;
}

void sui_message_update(SuiMessage *self){
    SuiMessageClass *class;

    g_return_if_fail(SUI_IS_MESSAGE(self));
    class = SUI_MESSAGE_GET_CLASS(self);
    g_return_if_fail (class->update);

    class->update(self);
}

void sui_message_label_on_popup(GtkLabel *label, GtkMenu *menu, gpointer user_data){
    int n;
    GSList *lst;
    GtkMenuItem *copy_menu_item;
    GtkMenuItem *forward_menu_item;
    GtkMenu *forward_submenu;
    SuiMessage *self;

    self = SUI_MESSAGE(user_data);

    /* Create menuitem copy_menu_item */
    copy_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Copy message")));
    gtk_widget_show(GTK_WIDGET(copy_menu_item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(copy_menu_item));
    g_signal_connect(copy_menu_item, "activate",
                G_CALLBACK(copy_menu_item_on_activate), self);

    /* Create menuitem forward_menu_item */
    forward_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Forward to...")));
    gtk_widget_show(GTK_WIDGET(forward_menu_item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(forward_menu_item));

    /* Create submenu of forward_menu_item */
    n = 0;
    forward_submenu = GTK_MENU(gtk_menu_new());
    lst = NULL;
    // FIXME
    while (lst){
        GtkMenuItem *item;

        item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                    sui_buffer_get_name(SUI_BUFFER(lst->data))));
        gtk_widget_show(GTK_WIDGET(item));
        g_signal_connect(item, "activate",
                G_CALLBACK(froward_submenu_item_on_activate), self);
        gtk_menu_shell_append(GTK_MENU_SHELL(forward_submenu), GTK_WIDGET(item));

        n++;
        lst = g_slist_next(lst);
    }

    if (n > 0) {
        gtk_menu_item_set_submenu(forward_menu_item, GTK_WIDGET(forward_submenu));
    } else {
        g_object_ref_sink(forward_submenu); // remove the floating reference
        g_object_unref(forward_submenu);
    }
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_message_real_update(SuiMessage *self){
    gtk_label_set_markup(self->message_label, self->ctx->dcontent);
}

static void sui_message_set_ctx(SuiMessage *self, void *ctx){
    self->ctx = ctx;
}

static void copy_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char* copied;
    GtkClipboard *cb;
    SuiMessage *self;

    self = SUI_MESSAGE(user_data);

    // Get the clipboard object
    cb = gtk_widget_get_clipboard(GTK_WIDGET(self), GDK_SELECTION_CLIPBOARD);
    copied = srn_message_to_string(self->ctx);
    gtk_clipboard_set_text(cb, copied, -1);
    g_free(copied);
}

static void froward_submenu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char *sel_msg;
    char *line;
    SuiBuffer *buf;
    SuiMessage *self;

    self = SUI_MESSAGE(user_data);
    buf = self->buf;

    sel_msg = label_get_selection(self->message_label);
    if (!sel_msg){
        return;
    }

    line = strtok(sel_msg, "\n");
    while (line){
        char *fwd;
        GVariantDict *params;

        fwd = g_strdup_printf(_("%1$s <fwd %2$s@%3$s>"), line,
                self->ctx->user->srv_user->nick, self->ctx->chat->name);
        line = strtok(NULL, "\n");

        params = g_variant_dict_new(NULL);
        g_variant_dict_insert(params, "message", SUI_EVENT_PARAM_STRING, fwd);

        sui_buffer_event_hdr(buf, SUI_EVENT_SEND, params);

        g_variant_dict_unref(params);
        g_free(fwd);
    }

    g_free(sel_msg);
}

/**
 * @brief Get the selected text (utf-8 supported) of `label`.
 * If no text was selected, return all of the text in this label.
 * If there is any '\n'(newline) in the text, strip it.
 *
 * @return A allocated (char *), it should be freed by `free()`
 */
static char* label_get_selection(GtkLabel *label){
    int start, end;
    const char *msg;
    char *sel_msg;
    if (!label) return NULL;

    msg = gtk_label_get_text(label);

    if (gtk_label_get_selection_bounds(label, &start, &end)){
        sel_msg = g_utf8_substring(msg, start, end);
    } else {
        sel_msg = g_strdup(msg);
    }

    return sel_msg;
}

