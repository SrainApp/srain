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

#include <gtk/gtk.h>
#include <string.h>

#include "log.h"

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_window.h"
#include "sui_buffer.h"

static void nick_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    const char *nick;
    GVariantDict *params;
    SuiBuffer *buf;

    buf = sui_common_get_cur_buffer();
    nick = user_data;

    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "nick", SUI_EVENT_PARAM_STRING, nick);

    if (strcmp(gtk_widget_get_name(widget), "whois_menu_item") == 0){
        sui_buffer_event_hdr(buf, SUI_EVENT_WHOIS, params);
    }
    else if (strcmp(gtk_widget_get_name(widget), "ignore_menu_item") == 0){
        sui_buffer_event_hdr(buf, SUI_EVENT_IGNORE, params);
    }
    else if (strcmp(gtk_widget_get_name(widget), "kick_menu_item") == 0){
        sui_buffer_event_hdr(buf, SUI_EVENT_KICK, params);
    }
    else if (strcmp(gtk_widget_get_name(widget), "chat_menu_item") == 0){
        sui_buffer_event_hdr(buf, SUI_EVENT_QUERY, params);
    }
    else if (strcmp(gtk_widget_get_name(widget), "invite_submenu_item") == 0){
        sui_buffer_event_hdr(buf, SUI_EVENT_INVITE, params);
    }
    else {
        ERR_FR("Unknown menu item: %s", gtk_widget_get_name(widget));
    }

    g_variant_dict_unref(params);
}

void nick_menu_popup(GtkWidget *widget, GdkEventButton *event, const char *nick){
    int n;
    GList *lst;
    GtkBuilder *builder;
    GtkMenu *nick_menu;
    GtkMenuItem *whois_menu_item;
    GtkMenuItem *ignore_menu_item;
    GtkMenuItem *kick_menu_item;
    GtkMenuItem *chat_menu_item;
    GtkMenuItem *invite_menu_item;
    GtkMenu *invite_submenu;
    SuiBuffer *buf;

    LOG_FR("%s", nick);

    builder = gtk_builder_new_from_resource ("/im/srain/Srain/nick_menu.glade");

    nick_menu = (GtkMenu *)gtk_builder_get_object(builder, "nick_menu");
    whois_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "whois_menu_item");
    ignore_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "ignore_menu_item");
    kick_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "kick_menu_item");
    chat_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "chat_menu_item");
    invite_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "invite_menu_item");

    g_signal_connect(whois_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
    g_signal_connect(ignore_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
    g_signal_connect(kick_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
    g_signal_connect(chat_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);

    /* Create subitem of invite_menu_item */
    buf = sui_common_get_cur_buffer();
    buf = NULL;
    goto FIN;

    n = 0;
    invite_submenu = GTK_MENU(gtk_menu_new());
    lst = NULL;
    // FIXME
    while (lst){
        GtkMenuItem *item;

        item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                    sui_buffer_get_name(lst->data)));
        gtk_widget_show(GTK_WIDGET(item));
        gtk_widget_set_name(GTK_WIDGET(item), "invite_submenu_item");
        g_signal_connect(item, "activate",
                G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
        gtk_menu_shell_append(GTK_MENU_SHELL(invite_submenu), GTK_WIDGET(item));

        n++;
        lst = g_list_next(lst);
    }

    if (n > 0) {
        gtk_menu_item_set_submenu(invite_menu_item, GTK_WIDGET(invite_submenu));
    } else {
        g_object_ref_sink(invite_submenu); // remove the floating reference
        g_object_unref(invite_submenu);
    }
FIN:
    gtk_menu_popup(nick_menu, NULL, NULL, NULL, NULL,
            event->button, event->time);
    g_object_unref(builder);
}
