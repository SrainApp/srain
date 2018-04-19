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
 * @file sui_buffer.c
 * @brief Chat buffer widget
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */


#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "core/core.h"
#include "sui/sui.h"

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_buffer.h"
#include "srain_entry_completion.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_msg.h"
#include "theme.h"

#include "log.h"
#include "i18n.h"
#include "utils.h"

struct _SuiBuffer {
    GtkBox parent;

    SrnChat *ctx;
    SuiBufferEvents *events;
    SuiBufferConfig *cfg;

    /* Topic */
    GtkRevealer *topic_revealer;
    GtkLabel *topic_label;

    /* Menus */
    GtkMenu *menu;
    GtkCheckMenuItem *topic_menu_item;
    GtkCheckMenuItem *user_list_menu_item;
    GtkMenuItem *close_menu_item;
    GtkMenuItem *leave_menu_item;
    GtkMenuItem *disconn_menu_item;
    GtkMenuItem *quit_menu_item;

    /* Message list */
    GtkBox *msg_list_box;
    SrainMsgList *msg_list;

    /* User list */
    GtkRevealer *user_list_revealer;
    SrainUserList *user_list;

    GtkTextBuffer *input_text_buffer;
};

struct _SuiBufferClass {
    GtkBoxClass parent_class;
};

static void sui_buffer_set_ctx(SuiBuffer *self, void *ctx);
static void sui_buffer_set_events(SuiBuffer *self, SuiBufferEvents *events);

static void topic_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);
static void user_list_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);
static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void leave_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data);
static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_CTX = 1,
  PROP_EVENTS,
  PROP_CONFIG,
  PROP_NAME,
  PROP_REMARK,
  N_PROPERTIES
};

G_DEFINE_TYPE(SuiBuffer, sui_buffer, GTK_TYPE_BOX);

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void sui_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiBuffer *self = SUI_BUFFER(object);

  switch (property_id){
    case PROP_CTX:
      sui_buffer_set_ctx(self, g_value_get_pointer(value));
      break;
    case PROP_EVENTS:
      sui_buffer_set_events(self, g_value_get_pointer(value));
      break;
    case PROP_CONFIG:
      sui_buffer_set_config(self, g_value_get_pointer(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_buffer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiBuffer *self = SUI_BUFFER(object);

  switch (property_id){
    case PROP_CTX:
      g_value_set_pointer(value, sui_buffer_get_ctx(self));
      break;
    case PROP_EVENTS:
      g_value_set_pointer(value, sui_buffer_get_events(self));
      break;
    case PROP_CONFIG:
      g_value_set_pointer(value, sui_buffer_get_config(self));
      break;
    case PROP_NAME:
      g_value_set_string(value, sui_buffer_get_name(self));
      break;
    case PROP_REMARK:
      g_value_set_string(value, sui_buffer_get_remark(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_buffer_init(SuiBuffer *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    /* Init user list*/
    self->user_list = srain_user_list_new();
    // Hide user_list for avoiding warning
    gtk_widget_hide(GTK_WIDGET(self->user_list));
    gtk_container_add(GTK_CONTAINER(self->user_list_revealer), // FIXME
            GTK_WIDGET(self->user_list));

    /* Init msg list */
    self->msg_list = srain_msg_list_new();
    gtk_box_pack_start(self->msg_list_box, GTK_WIDGET(self->msg_list),
            TRUE, TRUE, 0);
    gtk_widget_show(GTK_WIDGET(self->msg_list));

    g_signal_connect(self->topic_label, "activate-link",
            G_CALLBACK(activate_link), self);
    g_signal_connect(self->topic_menu_item, "toggled",
            G_CALLBACK(topic_menu_item_on_toggled), self);
    g_signal_connect(self->user_list_menu_item, "toggled",
            G_CALLBACK(user_list_menu_item_on_toggled), self);
    g_signal_connect(self->close_menu_item, "activate",
            G_CALLBACK(close_menu_item_on_activate), self);
    g_signal_connect(self->leave_menu_item, "activate",
            G_CALLBACK(leave_menu_item_on_activate), self);
    g_signal_connect(self->disconn_menu_item, "activate",
            G_CALLBACK(disconn_menu_item_on_activate),self);
    g_signal_connect(self->quit_menu_item, "activate",
            G_CALLBACK(quit_menu_item_on_activate),self);
}

static void sui_buffer_constructed(GObject *object){
    SuiBuffer *self;

    self = SUI_BUFFER(object);
    switch(self->ctx->type){
        case SRN_CHAT_TYPE_SERVER:
            gtk_widget_show(GTK_WIDGET(self->quit_menu_item));
            gtk_widget_show(GTK_WIDGET(self->disconn_menu_item));
            break;
        case SRN_CHAT_TYPE_CHANNEL:
            gtk_widget_show(GTK_WIDGET(self->leave_menu_item));
            break;
        case SRN_CHAT_TYPE_DIALOG:
            gtk_widget_show(GTK_WIDGET(self->close_menu_item));
            break;
        default:
            g_warn_if_reached();
    }

    sui_buffer_show_topic(self, self->cfg->show_topic);
    sui_buffer_show_user_list(self, self->cfg->show_user_list);

    G_OBJECT_CLASS(sui_buffer_parent_class)->constructed(object);
}

static void sui_buffer_finalize(GObject *object){
    G_OBJECT_CLASS(sui_buffer_parent_class)->finalize(object);
}

static void sui_buffer_class_init(SuiBufferClass *class){
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = sui_buffer_constructed;
    object_class->finalize = sui_buffer_finalize;
    object_class->set_property = sui_buffer_set_property;
    object_class->get_property = sui_buffer_get_property;

    /* Install properties */
    obj_properties[PROP_NAME] =
        g_param_spec_string("name",
                "Name",
                "Name of buffer.",
                NULL  /* default value */,
                G_PARAM_READABLE);

    obj_properties[PROP_REMARK] =
        g_param_spec_string("remark",
                "Remark",
                "Remark of buffer.",
                NULL  /* default value */,
                G_PARAM_READABLE);

    obj_properties[PROP_CTX] =
        g_param_spec_pointer("context",
                "Context",
                "Context of buffer.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_EVENTS] =
        g_param_spec_pointer("events",
                "Events",
                "Event callbacks of buffer.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_CONFIG] =
        g_param_spec_pointer("config",
                "Config",
                "Configuration of buffer.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);

    widget_class = GTK_WIDGET_CLASS(class);

    /* Bind child */
    gtk_widget_class_set_template_from_resource(
            widget_class, "/im/srain/Srain/buffer.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_revealer);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_label);

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, menu);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_menu_item);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, user_list_menu_item);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, close_menu_item);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, leave_menu_item);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, disconn_menu_item);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, quit_menu_item);

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, msg_list_box);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, user_list_revealer);

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, input_text_buffer);
}


/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiBuffer* sui_buffer_new(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg){
    return g_object_new(SUI_TYPE_BUFFER,
            "context", ctx,
            "events", events,
            "config", cfg,
            NULL);
}

void sui_buffer_insert_text(SuiBuffer *self, const char *text, int line, int offset){
    GtkTextMark *insert;
    GtkTextIter iter;

    insert = gtk_text_buffer_get_insert(self->input_text_buffer);
    gtk_text_buffer_get_iter_at_mark(self->input_text_buffer, &iter, insert);
    if (line == -1){ // Current line
        line = gtk_text_iter_get_line(&iter);
    }
    if (offset == -1) {
        offset = gtk_text_iter_get_line_offset(&iter);
    }

    gtk_text_buffer_get_iter_at_line_offset(self->input_text_buffer, &iter, line, offset);
    gtk_text_buffer_insert(self->input_text_buffer, &iter, text, -1);
}

void sui_buffer_show_topic(SuiBuffer *self, bool isshow){
    gtk_check_menu_item_set_active(self->topic_menu_item, isshow);
}

void sui_buffer_show_user_list(SuiBuffer *self, bool show){
    gtk_check_menu_item_set_active(self->user_list_menu_item, show);
}

const char* sui_buffer_get_name(SuiBuffer *self){
    return self->ctx->name;
}

const char* sui_buffer_get_remark(SuiBuffer *self){
    return self->ctx->srv->cfg->name;
}

// sui_buffer_set_events is static

SuiBufferEvents* sui_buffer_get_events(SuiBuffer *self){
    return self->events;
}

void sui_buffer_set_config(SuiBuffer *self, SuiBufferConfig *cfg){
    self->cfg = cfg;
    sui_buffer_show_topic(self, self->cfg->show_topic);
}

SuiBufferConfig* sui_buffer_get_config(SuiBuffer *self){
    return self->cfg;
}

// sui_buffer_set_ctx() is static

void* sui_buffer_get_ctx(SuiBuffer *self){
    return self->ctx;
}

void sui_buffer_set_topic(SuiBuffer *self, const char *topic){
    gtk_label_set_markup(self->topic_label, topic);
    gtk_check_menu_item_toggled(self->topic_menu_item);
}

void sui_buffer_set_topic_setter(SuiBuffer *self, const char *setter){
    gtk_widget_set_tooltip_text(GTK_WIDGET(self->topic_label), setter);
}

SrainMsgList* sui_buffer_get_msg_list(SuiBuffer *self){
    return self->msg_list;
}

GtkMenu* sui_buffer_get_menu(SuiBuffer *self){
    return self->menu;
}

SrainUserList* sui_buffer_get_user_list(SuiBuffer *self){
    return self->user_list;
}

GtkTextBuffer* sui_buffer_get_input_text_buffer(SuiBuffer *self){
    return self->input_text_buffer;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_buffer_set_ctx(SuiBuffer *self, void *ctx){
    self->ctx = ctx;
}

static void sui_buffer_set_events(SuiBuffer *self, SuiBufferEvents *events){
    self->events = events;
}

static void topic_menu_item_on_toggled(GtkWidget* widget, gpointer user_data){
    bool active;
    SuiBuffer *self = SUI_BUFFER(user_data);
    GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM(widget);

    active = gtk_check_menu_item_get_active(item);
    gtk_revealer_set_reveal_child(self->topic_revealer, active);

    // If topic is empty, do not show it anyway
    if (active && strlen(gtk_label_get_text(self->topic_label)) != 0){
        gtk_widget_show(GTK_WIDGET(self->topic_label));
    } else {
        gtk_widget_hide(GTK_WIDGET(self->topic_label));
    }
}

static void user_list_menu_item_on_toggled(GtkWidget* widget, gpointer user_data){
    bool active;
    SuiBuffer *self  = SUI_BUFFER(user_data);
    GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM(widget);

    active = gtk_check_menu_item_get_active(item);
    gtk_revealer_set_reveal_child(self->user_list_revealer, active);
    if (active){
        gtk_widget_show(GTK_WIDGET(self->user_list));
    } else {
        gtk_widget_hide(GTK_WIDGET(self->user_list));
    }
}

static void close_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiBuffer *self;

    self = user_data;
    sui_buffer_event_hdr(self, SUI_EVENT_UNQUERY, NULL);
}

static void leave_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiBuffer *self;

    self = user_data;
    sui_buffer_event_hdr(self, SUI_EVENT_PART, NULL);
}

static void disconn_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiBuffer *self;

    self = user_data;
    sui_buffer_event_hdr(self, SUI_EVENT_DISCONNECT, NULL);
}

static void quit_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    SuiBuffer *self;

    self = user_data;
    sui_buffer_event_hdr(self, SUI_EVENT_QUIT, NULL);
}

