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

static void topic_menu_item_on_toggled(GtkWidget* widget, gpointer user_data);
static gboolean entry_on_key_press(gpointer user_data, GdkEventKey *event);
static gboolean upload_image_idle(GtkEntry *entry);
static void upload_image_async(GtkEntry *entry);
static void upload_image_button_on_click(GtkWidget *widget, gpointer user_data);
static void input_entry_on_activate(SrainBuffer *buffer);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_SESSION = 1,
  PROP_NAME,
  PROP_REMARK,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SrainBuffer, srain_buffer, GTK_TYPE_BOX);

static void srain_buffer_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SrainBuffer *self = SRAIN_BUFFER(object);

  switch (property_id){
    case PROP_SESSION:
      srain_buffer_set_session(self, g_value_get_pointer(value));
      break;
    case PROP_NAME:
      srain_buffer_set_name(self, g_value_get_string(value));
      break;
    case PROP_REMARK:
      srain_buffer_set_remark(self, g_value_get_string(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void srain_buffer_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SrainBuffer *self = SRAIN_BUFFER(object);

  switch (property_id){
    case PROP_SESSION:
      g_value_set_pointer(value, srain_buffer_get_session(self));
      break;
    case PROP_NAME:
      g_value_set_string(value, srain_buffer_get_name(self));
      break;
    case PROP_REMARK:
      g_value_set_string(value, srain_buffer_get_remark(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void srain_buffer_init(SrainBuffer *self){
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

static void srain_buffer_finalize(GObject *object){
    SrainBuffer *self = SRAIN_BUFFER(object);

    g_free(self->name);
    g_free(self->remark);

    G_OBJECT_CLASS(srain_buffer_parent_class)->finalize(object);
}

static void srain_buffer_class_init(SrainBufferClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    /* Overwrite callbacks */
    object_class->finalize = srain_buffer_finalize;
    object_class->set_property = srain_buffer_set_property;
    object_class->get_property = srain_buffer_get_property;

    /* Install properties */
    obj_properties[PROP_SESSION] =
        g_param_spec_pointer("session",
                "Session",
                "Session of the buffer.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    obj_properties[PROP_NAME] =
        g_param_spec_string("name",
                "Name",
                "Name of the buffer.",
                NULL  /* default value */,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    obj_properties[PROP_REMARK] =
        g_param_spec_string("remark",
                "Remark",
                "Remark of the buffer.",
                NULL  /* default value */,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);

    /* Bind child */
    gtk_widget_class_set_template_from_resource(widget_class, "/org/gtk/srain/buffer.glade");

    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, name_label);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, remark_label);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, menu);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, topic_revealer);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, topic_label);

    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, msg_list_box);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, user_list_revealer);

    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, nick_label);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, input_entry);
    gtk_widget_class_bind_template_child(widget_class, SrainBuffer, upload_image_button);
}


/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SrainBuffer* srain_buffer_new(SuiSession *sui, const char *name, const char *remark){
    SrainBuffer *buffer;

    buffer = g_object_new(SRAIN_TYPE_BUFFER,
            "session", sui,
            "name", name,
            "remark", remark,
            NULL);

    return buffer;
}

/**
 * @brief Insert text into a SrainBuffer's input entry
 *
 * @param buffer
 * @param text
 * @param pos If the pos = -1, insert at current position
 */
void srain_buffer_insert_text(SrainBuffer *buffer, const char *text, int pos){
    GtkEntryBuffer *buf;

    buf = gtk_entry_get_buffer(buffer->input_entry);
    if (pos == -1)
        pos = gtk_editable_get_position(GTK_EDITABLE(buffer->input_entry));

    gtk_entry_buffer_insert_text(buf, pos, text, -1);
    gtk_editable_set_position(GTK_EDITABLE(buffer->input_entry),
            pos + strlen(text));
}

void srain_buffer_fcous_entry(SrainBuffer *buffer){
    gtk_widget_grab_focus(GTK_WIDGET(buffer->input_entry));
}

void srain_buffer_set_name(SrainBuffer *buffer, const char *name){
    str_assign(&buffer->name, name);
    gtk_label_set_text(buffer->name_label, name);
    gtk_widget_set_name(GTK_WIDGET(buffer), buffer->name);

}

const char* srain_buffer_get_name(SrainBuffer *buffer){
    return buffer->name;
}

void srain_buffer_set_remark(SrainBuffer *buffer, const char *remark){
    str_assign(&buffer->remark, remark);
    gtk_label_set_text(buffer->remark_label, remark);
}

const char* srain_buffer_get_remark(SrainBuffer *buffer){
    return buffer->remark;
}

void srain_buffer_set_nick(SrainBuffer *buffer, const char *nick){
    gtk_label_set_text(buffer->nick_label, nick);
}

const char* srain_buffer_get_nick(SrainBuffer *buffer){
    return gtk_label_get_text(buffer->nick_label);
}

void srain_buffer_set_topic(SrainBuffer *buffer, const char *topic){
    gtk_label_set_markup(buffer->topic_label, topic);
    gtk_check_menu_item_toggled(buffer->topic_menu_item);
}

void srain_buffer_set_topic_setter(SrainBuffer *buffer, const char *setter){
    gtk_widget_set_tooltip_text(GTK_WIDGET(buffer->topic_label), setter);
}

void srain_buffer_show_topic(SrainBuffer *buffer, bool isshow){
    gtk_check_menu_item_set_active(buffer->topic_menu_item, isshow);
}

SrainMsgList* srain_buffer_get_msg_list(SrainBuffer *buffer){
    return buffer->msg_list;
}

SrainEntryCompletion* srain_buffer_get_entry_completion(SrainBuffer *buffer){
    return buffer->completion;
}

GtkMenu* srain_buffer_get_menu(SrainBuffer *buffer){
    return buffer->menu;
}

void srain_buffer_set_session(SrainBuffer *buffer, SuiSession *session){
    buffer->session = session;
}

SuiSession *srain_buffer_get_session(SrainBuffer *buffer){
    return buffer->session;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void topic_menu_item_on_toggled(GtkWidget* widget, gpointer user_data){
    bool active;
    SrainBuffer *self = SRAIN_BUFFER(user_data);
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
    SrainBuffer *buffer;

    buffer = user_data;
    switch (event->keyval){
        case GDK_KEY_Down:
            // TODO: use up/down to switch history message?
            srain_msg_list_scroll_down(buffer->msg_list, 30);
            break;
        case GDK_KEY_Up:
            srain_msg_list_scroll_up(buffer->msg_list, 30);
            break;
        case GDK_KEY_Tab:
            srain_entry_completion_complete(buffer->completion);
            break;
        case GDK_KEY_n:
            if (event->state & GDK_CONTROL_MASK){
                srain_entry_completion_complete(buffer->completion);
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

static void input_entry_on_activate(SrainBuffer *buffer){
    const char *input;
    GVariantDict *params;
    SrnRet ret;

    input = gtk_entry_get_text(buffer->input_entry);

    if (str_is_empty(input)) goto RET;

    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "message", SUI_EVENT_PARAM_STRING, input);

    ret = sui_event_hdr(srain_buffer_get_session(buffer), SUI_EVENT_SEND, params);

    g_variant_dict_unref(params);

    if (!RET_IS_OK(ret)){
        return; // Don't empty the input_entry if error occured
    }

RET:
    gtk_entry_set_text(buffer->input_entry, "");

    return;
}
