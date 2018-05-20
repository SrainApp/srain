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
 * @file sui_side_bar_item.c
 * @brief Item of SuiSideBar
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-07
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "sui_side_bar_item.h"

#include "meta.h"
#include "log.h"
#include "utils.h"

struct _SuiSideBarItem {
    GtkBox parent;

    unsigned long update_time;

    GtkImage *image;
    GtkLabel *title_label;
    GtkLabel *subtitle_label;
    GtkLabel *recent_message_label;
    GtkLabel *unread_count_label;
};

struct _SuiSideBarItemClass {
    GtkBoxClass parent_class;
};

static char *strip_markup_tag(const char *markup);
static void contact_text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiSideBarItem, sui_side_bar_item, GTK_TYPE_BOX);
static void sui_side_bar_item_init(SuiSideBarItem *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void sui_side_bar_item_class_init(SuiSideBarItemClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/side_bar_item.glade");
    gtk_widget_class_bind_template_child(widget_class, SuiSideBarItem, image);
    gtk_widget_class_bind_template_child(widget_class, SuiSideBarItem, title_label);
    gtk_widget_class_bind_template_child(widget_class, SuiSideBarItem, subtitle_label);
    gtk_widget_class_bind_template_child(widget_class, SuiSideBarItem, recent_message_label);
    gtk_widget_class_bind_template_child(widget_class, SuiSideBarItem, unread_count_label);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiSideBarItem *sui_side_bar_item_new(const char *title,
        const char *subtitle, const char *icon){
    SuiSideBarItem *self;

    self = g_object_new(SUI_TYPE_SIDE_BAR_ITEM, NULL);

    self->update_time = get_time_since_first_call_ms();
    gtk_label_set_text(self->title_label, title);
    gtk_label_set_text(self->subtitle_label, subtitle);
    gtk_image_set_from_icon_name(self->image, icon, GTK_ICON_SIZE_BUTTON);

    return self;
}

void sui_side_bar_item_update(SuiSideBarItem *self,
        const char *nick, const char *msg){
    char *text;
    GtkWidget *row;

    text = strip_markup_tag(msg);
    g_return_if_fail(text);

    if (nick){
        char *buf;

        buf = g_strdup_printf("%s: %s", nick, text);
        gtk_label_set_text(self->recent_message_label, buf);
        g_free(buf);
    } else {
        gtk_label_set_text(self->recent_message_label, text);
    }
    g_free(text);

    self->update_time = get_time_since_first_call_ms();

    /* Mark as chagned */
    row = gtk_widget_get_parent(self);
    g_return_if_fail(GTK_IS_LIST_BOX_ROW(row));
    gtk_list_box_row_changed(row);
}

void sui_side_bar_item_highlight(SuiSideBarItem *self){
    GtkStyleContext *style_context;

    style_context = gtk_widget_get_style_context(GTK_WIDGET(self->unread_count_label));
    gtk_style_context_add_class(style_context, "highlighted");
}

void sui_side_bar_item_inc_count(SuiSideBarItem *self){
    int count;
    char *buf;
    GtkStyleContext *ctx;

    count = atoi(gtk_label_get_text(self->unread_count_label));
    buf = g_strdup_printf("%d", count + 1);
    gtk_label_set_text(self->unread_count_label, buf);
    g_free(buf);

    ctx = gtk_widget_get_style_context(GTK_WIDGET(self->unread_count_label));
    gtk_style_context_add_class(ctx, "message-count-label");
}

void sui_side_bar_item_clear_count(SuiSideBarItem *self){
    GtkStyleContext *ctx;

    gtk_label_set_text(self->unread_count_label, "");

    ctx = gtk_widget_get_style_context(GTK_WIDGET(self->unread_count_label));
    gtk_style_context_remove_class(ctx, "message-count-label");
    gtk_style_context_remove_class(ctx, "highlighted");
}

unsigned long sui_side_bar_item_get_update_time(SuiSideBarItem *self){
    return self->update_time;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static char *strip_markup_tag(const char *markup){
    char *text;
    const GMarkupParser parser = {NULL, NULL, contact_text, NULL, NULL};
    GString *str;
    GError *err;
    GMarkupParseContext *ctx;

    str = g_string_new(NULL);
    ctx = g_markup_parse_context_new(&parser, 0, &str, NULL);

    /* `markup` should be a vaild xml document, so add a root tag for it
     * ref: https://github.com/GNOME/gtk/blob/master/gtk/gtklabel.c#L2586
     */
    g_markup_parse_context_parse(ctx, "<markup>", -1, NULL);

    err = NULL;
    g_markup_parse_context_parse(ctx, markup, -1, &err);
    if (err){
        ERR_FR("%s", err->message);
    }

    g_markup_parse_context_parse(ctx, "</markup>", -1, NULL);
    g_markup_parse_context_end_parse(ctx, NULL);
    g_markup_parse_context_free(ctx);

    text = str->str;
    g_string_free(str, FALSE);

    return text;
}

static void contact_text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error){
    GString **str;

    str = user_data;
    *str = g_string_append_len(*str, text, text_len);
}
