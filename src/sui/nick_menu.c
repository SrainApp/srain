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

#include "i18n.h"
#include "log.h"

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_window.h"
#include "sui_buffer.h"
#include "gtk_compat.h"

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

static GtkWidget *new_nick_menu_item(const char *name, const char *label,
        const char *nick){
    GtkWidget *item;

    item = srn_gtk_menu_item_new_with_mnemonic(label);
    gtk_widget_set_name(item, name);
    srn_gtk_widget_show(item);
    srn_gtk_menu_item_connect_activate(item,
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);

    return item;
}

void nick_menu_popup(GtkWidget *widget, const char *nick){
    int n;
    GList *lst;
    GtkWidget *nick_menu;
    GtkWidget *whois_menu_item;
    GtkWidget *ignore_menu_item;
    GtkWidget *kick_menu_item;
    GtkWidget *chat_menu_item;
    GtkWidget *invite_menu_item;
#if GTK_MAJOR_VERSION < 4
    GtkWidget *invite_submenu;
#endif

#if GTK_MAJOR_VERSION >= 4
    nick_menu = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    nick_menu = gtk_menu_new();
#endif
    whois_menu_item = new_nick_menu_item("whois_menu_item", _("_Whois"), nick);
    ignore_menu_item = new_nick_menu_item("ignore_menu_item", _("_Ignore"), nick);
    kick_menu_item = new_nick_menu_item("kick_menu_item", _("_Kick"), nick);
    chat_menu_item = new_nick_menu_item("chat_menu_item", _("_Chat"), nick);
    invite_menu_item = srn_gtk_menu_item_new_with_mnemonic(_("Invite to..."));
    gtk_widget_set_name(invite_menu_item, "invite_menu_item");
    srn_gtk_widget_show(invite_menu_item);

    srn_gtk_menu_append(nick_menu, whois_menu_item);
    srn_gtk_menu_append(nick_menu, ignore_menu_item);
    srn_gtk_menu_append(nick_menu, kick_menu_item);
    srn_gtk_menu_append(nick_menu, chat_menu_item);
    srn_gtk_menu_append(nick_menu, invite_menu_item);

    /* Create subitems for invite_menu_item */
    n = 0;
    lst = sui_server_buffer_get_buffer_list(sui_common_get_cur_server_buffer());
#if GTK_MAJOR_VERSION < 4
    invite_submenu = gtk_menu_new();
#endif
    while (lst){
        GtkWidget *item;

        item = srn_gtk_menu_item_new_with_label(sui_buffer_get_name(lst->data));
        srn_gtk_widget_show(item);
        gtk_widget_set_name(item, "invite_submenu_item");
        srn_gtk_menu_item_connect_activate(item,
                G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
#if GTK_MAJOR_VERSION >= 4
        srn_gtk_menu_append(nick_menu, item);
#else
        srn_gtk_menu_append(invite_submenu, item);
#endif

        n++;
        lst = g_list_next(lst);
    }

#if GTK_MAJOR_VERSION >= 4
    if (n == 0) {
        gtk_widget_set_sensitive(invite_menu_item, FALSE);
    }
#else
    if (n > 0) {
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(invite_menu_item), invite_submenu);
    } else {
        g_object_ref_sink(invite_submenu); // remove the floating reference
        g_object_unref(invite_submenu);
    }
#endif

    sui_common_popup_panel(widget, nick_menu);
}
