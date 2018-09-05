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
 * @file sui_misc_message.c
 * @brief Subclass of SuiMessage, used for displaying system message and etc
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2018-04-06
 */

#include <gtk/gtk.h>

#include "sui_common.h"
#include "sui_misc_message.h"

#include "utils.h"
#include "i18n.h"

static void sui_misc_message_update(SuiMessage *_self);
static void sui_misc_message_update_side_bar_item(SuiMessage *_self,
        SuiSideBarItem *item);
static void sui_misc_message_compose_prev(SuiMessage *_self, SuiMessage *_prev);
static void sui_misc_message_compose_next(SuiMessage *_self, SuiMessage *_next);
static SuiNotification *sui_misc_message_new_notification(SuiMessage *_self);

static void sui_misc_message_set_style(SuiMiscMessage *self,
        SuiMiscMessageStyle style);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum {
  // 0 for PROP_NOME
  PROP_STYLE = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiMiscMessage, sui_misc_message, SUI_TYPE_MESSAGE);

static void sui_misc_message_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiMiscMessage *self;

  self = SUI_MISC_MESSAGE(object);

  switch (property_id){
    case PROP_STYLE:
      sui_misc_message_set_style(self, g_value_get_int(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_misc_message_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiMiscMessage *self;

  self = SUI_MISC_MESSAGE(object);

  switch (property_id){
    case PROP_STYLE:
      g_value_set_int(value, sui_misc_message_get_style(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_misc_message_init(SuiMiscMessage *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(SUI_MESSAGE(self)->message_label, "activate-link",
            G_CALLBACK(sui_common_activate_gtk_label_link), self);
    g_signal_connect(SUI_MESSAGE(self)->message_label, "populate-popup",
            G_CALLBACK(sui_message_label_on_popup), self);
}

static void sui_misc_message_class_init(SuiMiscMessageClass *class){
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;
    SuiMessageClass *message_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->set_property = sui_misc_message_set_property;
    object_class->get_property = sui_misc_message_get_property;

    obj_properties[PROP_STYLE] =
        g_param_spec_int("style",
                "Style",
                "Style of misc message.",
                SUI_MISC_MESSAGE_STYLE_NONE + 1,
                SUI_MISC_MESSAGE_STYLE_UNKNOWN - 1,
                SUI_MISC_MESSAGE_STYLE_NORMAL,
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/misc_message.glade");
    gtk_widget_class_bind_template_child(widget_class, SuiMessage, content_box);
    gtk_widget_class_bind_template_child(widget_class, SuiMessage, message_label);

    message_class = SUI_MESSAGE_CLASS(class);
    message_class->update = sui_misc_message_update;
    message_class->update_side_bar_item = sui_misc_message_update_side_bar_item;
    message_class->compose_prev = sui_misc_message_compose_prev;
    message_class->compose_next = sui_misc_message_compose_next;
    message_class->new_notification = sui_misc_message_new_notification;
}

static void sui_misc_message_update(SuiMessage *_self){
    char *full_time;
    SrnMessage *ctx;
    SuiMiscMessage *self;

    ctx = sui_message_get_ctx(_self);
    g_return_if_fail(ctx);
    self = SUI_MISC_MESSAGE(_self);

    full_time = sui_message_format_full_time(_self);
    g_return_if_fail(full_time);
    gtk_widget_set_tooltip_text(GTK_WIDGET(_self->message_label), full_time);
    g_free(full_time);

    SUI_MESSAGE_CLASS(sui_misc_message_parent_class)->update(_self);

    /* Override the content of message_label */
    if (self->style == SUI_MISC_MESSAGE_STYLE_ACTION) {
        char *action_msg;

        action_msg = g_strdup_printf("<b>%s</b> %s", ctx->dname, ctx->dcontent);
        gtk_label_set_markup(_self->message_label, action_msg);
        g_free(action_msg);
    }
}

static void sui_misc_message_update_side_bar_item(SuiMessage *_self,
        SuiSideBarItem *item){
    char *msg;
    SrnMessage *ctx;
    SuiMiscMessage *self;

    ctx = sui_message_get_ctx(_self);
    g_return_if_fail(ctx);
    self = SUI_MISC_MESSAGE(_self);

    switch (self->style) {
        case SUI_MISC_MESSAGE_STYLE_NORMAL:
            // Do not update
            break;
        case SUI_MISC_MESSAGE_STYLE_ACTION:
            msg = g_strdup_printf("%1$s %2$s", ctx->dname, ctx->dcontent);
            SUI_MESSAGE_CLASS(sui_misc_message_parent_class)->
                update_side_bar_item(_self, item);
            sui_side_bar_item_update(item, NULL, msg);
            g_free(msg);
            break;
        case SUI_MISC_MESSAGE_STYLE_ERROR:
            SUI_MESSAGE_CLASS(sui_misc_message_parent_class)->
                update_side_bar_item(_self, item);
            sui_side_bar_item_update(item, _("Error"), ctx->dcontent);
            break;
        default:
            g_warn_if_reached();
    }
}

static void sui_misc_message_compose_prev(SuiMessage *_self, SuiMessage *_prev){
    // Do nothing and not need to chain up
}

static void sui_misc_message_compose_next(SuiMessage *_self, SuiMessage *_next){
    // Do nothing and not need to chain up
}

static SuiNotification *sui_misc_message_new_notification(SuiMessage *_self){
    SuiMiscMessage *self;
    SuiNotification *notif;

    self = SUI_MISC_MESSAGE(_self);
    notif = SUI_MESSAGE_CLASS(sui_misc_message_parent_class)->new_notification(_self);
    g_return_val_if_fail(notif, NULL);

    if (self->style == SUI_MISC_MESSAGE_STYLE_ERROR){
        str_assign(&notif->icon, "srain-red");
    }

    return notif;
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiMiscMessage* sui_misc_message_new(void *ctx, SuiMiscMessageStyle style){
    return g_object_new(SUI_TYPE_MISC_MESSAGE,
            "context", ctx,
            "style", style,
            NULL);
}

SuiMiscMessageStyle sui_misc_message_get_style(SuiMiscMessage *self){
    return self->style;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_misc_message_set_style(SuiMiscMessage *self,
        SuiMiscMessageStyle style) {
    GtkStyleContext *style_context;
    const char *class;

    self->style = style;

    style_context = gtk_widget_get_style_context(GTK_WIDGET(self));
    switch (self->style) {
        case SUI_MISC_MESSAGE_STYLE_ACTION:
            class = "sui-misc-message-action";
            break;
        case SUI_MISC_MESSAGE_STYLE_ERROR:
            class = "sui-misc-message-error";
            break;
        default:
            return;
    }
    gtk_style_context_add_class(style_context, class);
}
