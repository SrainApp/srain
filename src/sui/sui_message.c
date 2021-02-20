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

#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "nick_menu.h"
#include "sui_buffer.h"
#include "sui_server_buffer.h"
#include "sui_chat_buffer.h"
#include "sui_message.h"
#include "sui_url_previewer.h"

#include "log.h"
#include "i18n.h"
#include "meta.h"
#include "utils.h"

static void sui_message_real_update(SuiMessage *self);
static void sui_message_real_update_side_bar_item(SuiMessage *self,
        SuiSideBarItem *item);
static void sui_message_real_compose_prev(SuiMessage *self, SuiMessage *prev);
static void sui_message_real_compose_next(SuiMessage *self, SuiMessage *next);
static SuiNotification* sui_message_real_new_notification(SuiMessage *self);

static void sui_message_set_ctx(SuiMessage *self, void *ctx);

static char* label_get_selection(GtkLabel *label);
static void copy_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void froward_submenu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void url_previewer_on_notify_content_type(GObject *object,
        GParamSpec *pspec, gpointer data);

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
    GtkStyleContext *style_context;

    style_context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style_context, "sui-message-head");
    gtk_style_context_add_class(style_context, "sui-message-tail");
}

static void sui_message_constructed(GObject *object){
    G_OBJECT_CLASS(sui_message_parent_class)->constructed(object);
}

static void sui_message_finalize(GObject *object){
    /* Widget will be automaticlly removed from GtkSizeGroup before it is
     * destroyed */
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
    class->update_side_bar_item = sui_message_real_update_side_bar_item;
    class->compose_prev = sui_message_real_compose_prev;
    class->compose_next = sui_message_real_compose_next;
    class->new_notification = sui_message_real_new_notification;
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

void sui_message_update_side_bar_item(SuiMessage *self, SuiSideBarItem *item){
    SuiMessageClass *class;

    g_return_if_fail(SUI_IS_MESSAGE(self));
    class = SUI_MESSAGE_GET_CLASS(self);
    g_return_if_fail (class->update_side_bar_item);

    class->update_side_bar_item(self, item);
}

void sui_message_compose_prev(SuiMessage *self, SuiMessage *prev){
    SuiMessageClass *class;

    g_return_if_fail(SUI_IS_MESSAGE(self));
    class = SUI_MESSAGE_GET_CLASS(self);
    g_return_if_fail(class->compose_prev);
    g_return_if_fail(!self->prev);

    // Only compose messages sent by same user.
    if (self->ctx->sender != prev->ctx->sender
            || g_ascii_strcasecmp(self->ctx->rendered_sender, prev->ctx->rendered_sender) != 0){
        return;
    }

    class->compose_prev(self, prev);
}

void sui_message_compose_next(SuiMessage *self, SuiMessage *next){
    SuiMessageClass *class;

    g_return_if_fail(SUI_IS_MESSAGE(self));
    class = SUI_MESSAGE_GET_CLASS(self);
    g_return_if_fail(class->compose_next);
    g_return_if_fail(!self->next);

    // Only compose messages sent by same user.
    if (self->ctx->sender != next->ctx->sender
            || g_ascii_strcasecmp(self->ctx->rendered_sender, next->ctx->rendered_sender) != 0){
        return;
    }

    class->compose_next(self, next);
}

SuiNotification* sui_message_new_notification(SuiMessage *self){
    SuiMessageClass *class;

    g_return_val_if_fail(SUI_IS_MESSAGE(self), NULL);
    class = SUI_MESSAGE_GET_CLASS(self);
    g_return_val_if_fail(class->new_notification, NULL);

    return class->new_notification(self);
}

const char* sui_message_get_time(SuiMessage *self){
    SrnMessage *ctx;

    ctx = sui_message_get_ctx(self);

    return ctx->rendered_short_time;
}

const char* sui_message_get_full_time(SuiMessage *self){
    SrnMessage *ctx;

    ctx = sui_message_get_ctx(self);

    return ctx->rendered_full_time;
}

bool sui_message_is_mentioned(SuiMessage *self){
    SrnMessage *ctx;

    ctx = sui_message_get_ctx(self);

    return ctx->mentioned;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_message_real_update(SuiMessage *self){
    GtkStyleContext *style_context;

    // Update message content
    gtk_label_set_markup(self->message_label, self->ctx->rendered_content);

    // Show url previewer if needed
    if (self->buf->cfg->preview_url) {
        GList *urls;
        urls = self->ctx->urls;

        for (GList *url = urls; url; url = g_list_next(url)) {
            bool found;

            found = FALSE;
            for (GList *child = gtk_widget_get_first_child(self->content_box);
                    child;
                    child = gtk_widget_get_next_sibling(child)) {
                if (SUI_IS_URL_PREVIEWER(child->data)) {
                    SuiUrlPreviewer *pvr;

                    pvr = SUI_URL_PREVIEWER(child->data);
                    if (g_strcmp0(url->data, sui_url_previewer_get_url(pvr)) == 0){
                        found = TRUE;
                    }
                }
            }
            if (!found) { // Create one if not found
                SuiUrlPreviewer *pvr;

                pvr = sui_url_previewer_new(url->data);
                if (sui_url_previewer_get_content_type(pvr) ==
                        SUI_URL_CONTENT_TYPE_UNSUPPORTED) {
                    g_object_ref_sink(pvr);
                    g_object_unref(pvr);
                } else {
                    // Add previewer on to message
                    gtk_box_pack_start(self->content_box, GTK_WIDGET(pvr),
                            TRUE, FALSE, 4);

                    // Auto preview if needed
                    if (self->buf->cfg->auto_preview_url){
                        // Hide previewer to prevent many loading previewers
                        // from appearing in the message.
                        gtk_widget_hide(GTK_WIDGET(pvr));
                        // Previewer will be shown via this callback when
                        // previewer's content type is supported
                        g_signal_connect(pvr, "notify::content-type",
                                G_CALLBACK(url_previewer_on_notify_content_type),
                                self->content_box);

                        sui_url_previewer_preview(pvr);
                    }

                }

            }
        }
    }
}

static void sui_message_real_update_side_bar_item(SuiMessage *self,
        SuiSideBarItem *item){
    sui_side_bar_item_update(item, self->ctx->rendered_sender, self->ctx->rendered_content);
    sui_side_bar_item_inc_count(item);
    if (self->ctx->mentioned){
        sui_side_bar_item_highlight(item);
    }
}

static void sui_message_real_compose_prev(SuiMessage *self, SuiMessage *prev){
    GtkStyleContext *style_context;

    g_return_if_fail(!self->prev);
    self->prev = prev;

    style_context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_remove_class(style_context, "sui-message-head");

    if (!self->size_group && !prev->size_group) {
        self->size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
        gtk_size_group_add_widget(self->size_group, GTK_WIDGET(self));
    } else if (self->size_group && !prev->size_group) {
        // Noting to do
    } else if (!self->size_group && prev->size_group) {
        self->size_group = prev->size_group;
        gtk_size_group_add_widget(self->size_group, GTK_WIDGET(self));
    } else {
        g_warn_if_reached();
    }
}

static void sui_message_real_compose_next(SuiMessage *self, SuiMessage *next){
    GtkStyleContext *style_context;

    g_return_if_fail(!self->next);
    self->next = next;

    style_context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_remove_class(style_context, "sui-message-tail");

    if (!self->size_group && !next->size_group) {
        self->size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
        gtk_size_group_add_widget(self->size_group, GTK_WIDGET(self));
    } else if (self->size_group && !next->size_group) {
        // Noting to do
    } else if (!self->size_group && next->size_group) {
        self->size_group = next->size_group;
        gtk_size_group_add_widget(self->size_group, GTK_WIDGET(self));
    } else {
        g_warn_if_reached();
    }
}

static SuiNotification* sui_message_real_new_notification(SuiMessage *self){
    SuiNotification *notif;
    char *title;

    notif = sui_notification_new();

    switch (self->ctx->chat->type) {
        case SRN_CHAT_TYPE_SERVER:
        case SRN_CHAT_TYPE_DIALOG:
            title = g_strdup_printf(_("%1$s @ %2$s"),
                    self->ctx->rendered_sender,
                    self->ctx->chat->srv->chat->name);
            break;
        case SRN_CHAT_TYPE_CHANNEL:
            title = g_strdup_printf(_("%1$s %2$s @ %3$s"),
                    self->ctx->rendered_sender,
                    self->ctx->chat->name,
                    self->ctx->chat->srv->chat->name);
            break;
        default:
            title = g_strdup_printf(_("Message from unknown chat"));
            g_warn_if_reached();
    }

    str_assign(&notif->id, "sui-message");
    str_assign(&notif->icon, PACKAGE_APPID);
    notif->title = title; // No need to copy
    str_assign(&notif->body, self->ctx->rendered_content);

    return notif;
}

static void sui_message_set_ctx(SuiMessage *self, void *ctx){
    self->ctx = ctx;
}

static void copy_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char* copied;
    GdkClipboard *cb;
    SuiMessage *self;

    self = SUI_MESSAGE(user_data);

    // Get the clipboard object
    cb = gtk_widget_get_clipboard(GTK_WIDGET(self));
    copied = srn_message_to_string(self->ctx);
    gdk_clipboard_set_text(cb, copied);
    g_free(copied);
}

static void url_previewer_on_notify_content_type(GObject *object,
        GParamSpec *pspec, gpointer data){
    SuiUrlPreviewer *pvr;

    pvr = SUI_URL_PREVIEWER(object);

    switch (sui_url_previewer_get_content_type(pvr)){
        case SUI_URL_CONTENT_TYPE_UNSUPPORTED:
        case SUI_URL_CONTENT_TYPE_UNKNOWN:
            break;
        default:
            gtk_widget_show(GTK_WIDGET(pvr));
            break;
    }
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

