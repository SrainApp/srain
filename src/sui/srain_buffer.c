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
 * @file srain_buffer.c
 * @brief Srain's chat panel widget, contains message list and input area
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */


#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "sui/sui.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "srain_buffer.h"
#include "srain_entry_completion.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_msg.h"
#include "theme.h"

#include "plugin.h"
#include "log.h"
#include "i18n.h"
#include "utils.h"

static void sui_buffer_set_events(SuiBuffer *self, SuiBufferEvents *events);
static void topic_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);
static gboolean entry_on_key_press(gpointer user_data, GdkEventKey *event);
static gboolean upload_image_idle(GtkEntry *entry);
static void upload_image_async(GtkEntry *entry);
static void upload_image_button_on_click(GtkWidget *widget, gpointer user_data);
static void input_entry_on_activate(SuiBuffer *self);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_NAME = 1,
  PROP_REMARK,
  PROP_EVENTS,
  PROP_CONFIG,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiBuffer, sui_buffer, GTK_TYPE_BOX);

static void sui_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiBuffer *self = SUI_BUFFER(object);

  switch (property_id){
    case PROP_NAME:
      sui_buffer_set_name(self, g_value_get_string(value));
      break;
    case PROP_REMARK:
      sui_buffer_set_remark(self, g_value_get_string(value));
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
    case PROP_NAME:
      g_value_set_string(value, sui_buffer_get_name(self));
      break;
    case PROP_REMARK:
      g_value_set_string(value, sui_buffer_get_remark(self));
      break;
    case PROP_EVENTS:
      g_value_set_pointer(value, sui_buffer_get_events(self));
      break;
    case PROP_CONFIG:
      g_value_set_pointer(value, sui_buffer_get_config(self));
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
    builder = gtk_builder_new_from_resource("/org/gtk/srain/buffer_menu.glade");
    self->topic_menu_item =
        (GtkCheckMenuItem *)gtk_builder_get_object(builder, "topic_menu_item");
    gtk_menu_shell_append(
            GTK_MENU_SHELL(self->menu),
            GTK_WIDGET(self->topic_menu_item));
    g_object_unref(builder);

    /* Init completion list */
    self->completion = srain_entry_completion_new(self->input_entry);

    /* Init msg list */
    self->msg_list = srain_msg_list_new();
    gtk_box_pack_start(self->msg_list_box, GTK_WIDGET(self->msg_list),
            TRUE, TRUE, 0);
    gtk_widget_show(GTK_WIDGET(self->msg_list));

    g_signal_connect(self->topic_menu_item, "toggled",
            G_CALLBACK(topic_menu_item_on_toggled), self);
    g_signal_connect(self->topic_label, "activate-link",
            G_CALLBACK(activate_link), self);
    g_signal_connect_swapped(self->input_entry, "activate",
            G_CALLBACK(input_entry_on_activate), self);
    g_signal_connect_swapped(self->input_entry, "key_press_event",
            G_CALLBACK(entry_on_key_press), self);
    g_signal_connect(self->upload_image_button, "clicked",
            G_CALLBACK(upload_image_button_on_click), self->input_entry);
}

static void sui_buffer_finalize(GObject *object){
    SuiBuffer *self = SUI_BUFFER(object);

    g_free(self->name);
    g_free(self->remark);

    G_OBJECT_CLASS(sui_buffer_parent_class)->finalize(object);
}

static void sui_buffer_class_init(SuiBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    /* Overwrite callbacks */
    object_class->finalize = sui_buffer_finalize;
    object_class->set_property = sui_buffer_set_property;
    object_class->get_property = sui_buffer_get_property;

    /* Install properties */
    obj_properties[PROP_NAME] =
        g_param_spec_string("name",
                "Name",
                "Name of self.",
                NULL  /* default value */,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    obj_properties[PROP_REMARK] =
        g_param_spec_string("remark",
                "Remark",
                "Remark of self.",
                NULL  /* default value */,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    obj_properties[PROP_EVENTS] =
        g_param_spec_pointer("event",
                "Events",
                "Event callbacks of self.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_CONFIG] =
        g_param_spec_pointer("config",
                "Config",
                "Configuration of self.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);

    /* Bind child */
    gtk_widget_class_set_template_from_resource(widget_class, "/org/gtk/srain/buffer.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, name_label);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, remark_label);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, menu);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_revealer);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, topic_label);

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, msg_list_box);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, user_list_revealer);

    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, nick_label);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, input_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiBuffer, upload_image_button);
}


/*****************************************************************************
 * Expored functions
 *****************************************************************************/

/**
 * @brief Insert text into a SuiBuffer's input entry
 *
 * @param self
 * @param text
 * @param pos If the pos = -1, insert at current position
 */
void sui_buffer_insert_text(SuiBuffer *self, const char *text, int pos){
    GtkEntryBuffer *buf;

    buf = gtk_entry_get_buffer(self->input_entry);
    if (pos == -1)
        pos = gtk_editable_get_position(GTK_EDITABLE(self->input_entry));

    gtk_entry_buffer_insert_text(buf, pos, text, -1);
    gtk_editable_set_position(GTK_EDITABLE(self->input_entry),
            pos + strlen(text));
}

void sui_buffer_fcous_entry(SuiBuffer *self){
    gtk_widget_grab_focus(GTK_WIDGET(self->input_entry));
}

void sui_buffer_set_name(SuiBuffer *self, const char *name){
    str_assign(&self->name, name);
    gtk_label_set_text(self->name_label, name);
    gtk_widget_set_name(GTK_WIDGET(self), self->name);

}

const char* sui_buffer_get_name(SuiBuffer *self){
    return self->name;
}

void sui_buffer_set_remark(SuiBuffer *self, const char *remark){
    str_assign(&self->remark, remark);
    gtk_label_set_text(self->remark_label, remark);
}

const char* sui_buffer_get_remark(SuiBuffer *self){
    return self->remark;
}

// sui_buffer_set_events is static

SuiBufferEvents* sui_buffer_get_events(SuiBuffer *self){
    return self->events;
}

void sui_buffer_set_config(SuiBuffer *self, SuiBufferConfig *cfg){
    // TODO: config
    self->cfg = cfg;
}

SuiBufferConfig* sui_buffer_get_config(SuiBuffer *self){
    return self->cfg;
}

void sui_buffer_set_ctx(SuiBuffer *self, void *ctx){
    self->ctx = ctx;
}

void* sui_buffer_get_ctx(SuiBuffer *self){
    return self->ctx;
}

void sui_buffer_set_nick(SuiBuffer *self, const char *nick){
    gtk_label_set_text(self->nick_label, nick);
}

const char* sui_buffer_get_nick(SuiBuffer *self){
    return gtk_label_get_text(self->nick_label);
}

void sui_buffer_set_topic(SuiBuffer *self, const char *topic){
    gtk_label_set_markup(self->topic_label, topic);
    gtk_check_menu_item_toggled(self->topic_menu_item);
}

void sui_buffer_set_topic_setter(SuiBuffer *self, const char *setter){
    gtk_widget_set_tooltip_text(GTK_WIDGET(self->topic_label), setter);
}

void sui_buffer_show_topic(SuiBuffer *self, bool isshow){
    gtk_check_menu_item_set_active(self->topic_menu_item, isshow);
}

SrainMsgList* sui_buffer_get_msg_list(SuiBuffer *self){
    return self->msg_list;
}

SrainEntryCompletion* sui_buffer_get_entry_completion(SuiBuffer *self){
    return self->completion;
}

GtkMenu* sui_buffer_get_menu(SuiBuffer *self){
    return self->menu;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

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

static gboolean entry_on_key_press(gpointer user_data, GdkEventKey *event){
    SuiBuffer *self;

    self = user_data;
    switch (event->keyval){
        case GDK_KEY_Down:
            // TODO: use up/down to switch history message?
            srain_msg_list_scroll_down(self->msg_list, 30);
            break;
        case GDK_KEY_Up:
            srain_msg_list_scroll_up(self->msg_list, 30);
            break;
        case GDK_KEY_Tab:
            srain_entry_completion_complete(self->completion);
            break;
        case GDK_KEY_n:
            if (event->state & GDK_CONTROL_MASK){
                srain_entry_completion_complete(self->completion);
                break;
            }
        default:
            return FALSE;
    }

    return TRUE;
}

static gboolean upload_image_idle(GtkEntry *entry){
    char *url;

    /* Check whether object is alive now */
    g_return_val_if_fail(GTK_IS_ENTRY(entry), FALSE);

    url = g_object_get_data(G_OBJECT(entry), "image-url");
    if (url){
        gtk_entry_set_text(entry, url);
        g_free(url);
    } else {
        gtk_entry_set_text(entry, _("Failed to upload image"));
    }

    g_object_set_data(G_OBJECT(entry), "image-url", NULL);
    gtk_widget_set_sensitive(GTK_WIDGET(entry), TRUE);

    /* NOTE: DON'T FORGET to return FALSE!!! */
    return FALSE;
}

static void upload_image_async(GtkEntry *entry){
    char *url;
    const char *filename;

    filename = gtk_entry_get_text(entry);
    url = plugin_upload(filename);

    g_object_set_data(G_OBJECT(entry), "image-url", url);
    gdk_threads_add_idle((GSourceFunc)upload_image_idle, entry);
}

static void upload_image_button_on_click(GtkWidget *widget, gpointer user_data){
    char *filename;
    GtkEntry *entry;
    GtkWindow *toplevel;

    entry = user_data;

    toplevel = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    filename = show_open_filechosser(toplevel);
    if (filename) {
        LOG_FR("filename: '%s'", filename);
        gtk_widget_set_sensitive(GTK_WIDGET(entry), FALSE);
        gtk_entry_set_text(entry, filename);

        g_thread_new(NULL, (GThreadFunc)upload_image_async, entry);

        g_free(filename);
    }
}

static void input_entry_on_activate(SuiBuffer *self){
    const char *input;
    GVariantDict *params;
    SrnRet ret;

    input = gtk_entry_get_text(self->input_entry);

    if (str_is_empty(input)) goto RET;

    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "message", SUI_EVENT_PARAM_STRING, input);

    ret = sui_buffer_event_hdr(self, SUI_EVENT_SEND, params);

    g_variant_dict_unref(params);

    if (!RET_IS_OK(ret)){
        return; // Don't empty the input_entry if error occured
    }

RET:
    gtk_entry_set_text(self->input_entry, "");

    return;
}
