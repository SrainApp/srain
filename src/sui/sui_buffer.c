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
 * @brief
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
#include "theme.h"
#include "sui_buffer.h"

#include "log.h"
#include "i18n.h"
#include "utils.h"

static GtkListStore* real_completion_func(SuiBuffer *self, const char *context);

static void sui_buffer_set_ctx(SuiBuffer *self, void *ctx);
static void sui_buffer_set_events(SuiBuffer *self, SuiBufferEvents *events);

static void topic_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);

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
    GtkBuilder *builder;

    gtk_widget_init_template(GTK_WIDGET(self));

    /* Init menus */
    builder = gtk_builder_new_from_resource("/im/srain/Srain/buffer_menu.glade");
    self->topic_menu_item =
        (GtkCheckMenuItem *)gtk_builder_get_object(builder, "topic_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(self->menu),
            GTK_WIDGET(self->topic_menu_item));
    g_object_unref(builder);

    /* Init msg list */
    self->msg_list = sui_message_list_new();
    gtk_box_pack_start(self->msg_list_box, GTK_WIDGET(self->msg_list),
            TRUE, TRUE, 0);
    gtk_widget_show(GTK_WIDGET(self->msg_list));

    /* Setup completion */
    self->completion = sui_completion_new(self->input_text_buffer);

    g_signal_connect(self->topic_label, "activate-link",
            G_CALLBACK(activate_link), self);
    g_signal_connect(self->topic_menu_item, "toggled",
            G_CALLBACK(topic_menu_item_on_toggled), self);
}

static void sui_buffer_constructed(GObject *object){
    SuiBuffer *self;

    self = SUI_BUFFER(object);

    sui_buffer_show_topic(self, self->cfg->show_topic);

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

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, menu);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_revealer);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_label);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, user_list_revealer);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, msg_list_box);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, input_text_buffer);

    class->completion_func = real_completion_func;
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

    g_return_if_fail(SUI_IS_BUFFER(self));

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
    g_return_if_fail(SUI_IS_BUFFER(self));

    gtk_check_menu_item_set_active(self->topic_menu_item, isshow);
}

/**
 * @brief sui_buffer_complete completes the contents of the input text
 * buffer of SuiBuffer
 *
 * @param self
 */
void sui_buffer_complete(SuiBuffer *self){
    sui_completion_complete(self->completion, sui_buffer_completion_func, self);
}

GtkTreeModel* sui_buffer_completion_func(const char *context, void *user_data) {
    SuiBuffer *self;
    SuiBufferClass *class;

    self = user_data;
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);
    class = SUI_BUFFER_GET_CLASS(self);
    g_return_val_if_fail(class->completion_func, NULL);

    return GTK_TREE_MODEL(class->completion_func(self, context));
}

const char* sui_buffer_get_name(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->ctx->name;
}

const char* sui_buffer_get_remark(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->ctx->srv->name;
}

// sui_buffer_set_events is static

SuiBufferEvents* sui_buffer_get_events(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->events;
}

void sui_buffer_set_config(SuiBuffer *self, SuiBufferConfig *cfg){
    g_return_if_fail(SUI_IS_BUFFER(self));

    self->cfg = cfg;
    sui_buffer_show_topic(self, self->cfg->show_topic);
}

SuiBufferConfig* sui_buffer_get_config(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->cfg;
}

// sui_buffer_set_ctx() is static

void* sui_buffer_get_ctx(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->ctx;
}

void sui_buffer_set_topic(SuiBuffer *self, const char *topic){
    g_return_if_fail(SUI_IS_BUFFER(self));

    gtk_label_set_markup(self->topic_label, topic);
    gtk_check_menu_item_toggled(self->topic_menu_item);
}

void sui_buffer_set_topic_setter(SuiBuffer *self, const char *setter){
    g_return_if_fail(SUI_IS_BUFFER(self));

    gtk_widget_set_tooltip_text(GTK_WIDGET(self->topic_label), setter);
}

SuiMessageList* sui_buffer_get_message_list(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->msg_list;
}

GtkMenu* sui_buffer_get_menu(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

    return self->menu;
}

GtkTextBuffer* sui_buffer_get_input_text_buffer(SuiBuffer *self){
    g_return_val_if_fail(SUI_IS_BUFFER(self), NULL);

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

static GtkListStore* real_completion_func(SuiBuffer *self, const char *context){
    return gtk_list_store_new(SUI_COMPLETION_N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING);
}
