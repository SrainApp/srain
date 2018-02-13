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
 * @file srain_stack_sidebar_item.c
 * @brief item class of SrainStackSidebar
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-07
 */


#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "srain_stack_sidebar_item.h"

#include "meta.h"
#include "log.h"
#include "utils.h"

struct _SrainStackSidebarItem {
    GtkBox parent;

    unsigned long update_time;

    GtkImage *image;
    GtkLabel *chat_label;
    GtkLabel *server_label;
    GtkLabel *recentmsg_label;
    GtkLabel *count_label;
};

struct _SrainStackSidebarItemClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainStackSidebarItem, srain_stack_sidebar_item, GTK_TYPE_BOX);

static char *markup_get_text(const char *markup);

static void srain_stack_sidebar_item_init(SrainStackSidebarItem *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_stack_sidebar_item_class_init(SrainStackSidebarItemClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/stack_sidebar_item.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, image);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, chat_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, recentmsg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, server_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, count_label);
}

SrainStackSidebarItem *srain_stack_sidebar_item_new(const char *name,
        const char *remark, const char *icon){
    SrainStackSidebarItem *item;

    item = g_object_new(SRAIN_TYPE_STACK_SIDEBAR_ITEM, NULL);

    item->update_time = get_time_since_first_call_ms();
    gtk_label_set_text(item->chat_label, name);
    gtk_label_set_text(item->server_label, remark);
    gtk_image_set_from_icon_name(item->image, icon, GTK_ICON_SIZE_BUTTON);

    return item;
}

void srain_stack_sidebar_item_recentmsg_update(
        SrainStackSidebarItem *item, const char *nick, const char *msg){
    char *text;

    item->update_time = get_time_since_first_call_ms();

    text = markup_get_text(msg);
    g_return_if_fail(text);

    if (nick){
        GString *buf;

        buf = g_string_new(NULL);
        g_string_printf(buf, "%s: %s", nick, text);
        gtk_label_set_text(item->recentmsg_label, buf->str);

        g_string_free(buf, TRUE);
    } else {
        gtk_label_set_text(item->recentmsg_label, text);
    }

    if (text){
        g_free(text);
    }
}

void srain_stack_sidebar_item_count_inc(SrainStackSidebarItem *item){
    int count;
    char buf[32];

    if ((count = atoi(gtk_label_get_text(item->count_label))) == 0){
        gtk_widget_set_name(GTK_WIDGET(item->count_label), "count_label");
    }
    snprintf(buf, 32, "%d", count + 1);

    gtk_label_set_text(item->count_label, buf);
}

void srain_stack_sidebar_item_count_clear(SrainStackSidebarItem *item){
    gtk_widget_set_name(GTK_WIDGET(item->count_label), "");
    gtk_label_set_text(item->count_label, "");
}

unsigned long srain_stack_sidebar_item_get_update_time(SrainStackSidebarItem *item){
    return item->update_time;
}

static void markup_contact_text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error){
    GString **str;

    str = user_data;
    *str = g_string_append_len(*str, text, text_len);
}

static char *markup_get_text(const char *markup){
    char *text;
    const GMarkupParser parser = {NULL, NULL, markup_contact_text, NULL, NULL};
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
