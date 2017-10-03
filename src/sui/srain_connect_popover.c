/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @file srain_connect_popover.c
 * @brief GtkPopover subclass for connection to server
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-09-16
 */

#include <stdlib.h>
#include <gtk/gtk.h>

#include "sui/sui.h"
#include "sui_event_hdr.h"
#include "srain_window.h"
#include "srain_connect_popover.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "utils.h"

#define SERVER_LIST_STORE_COL_NAME  0

#define PAGE_PREDEFINEED_SERVER     "perdefined_server_page"
#define PAGE_CUSTOM_SERVER          "custeom_server_page"

struct _SrainConnectPopover {
    GtkPopover parent;

    GtkStack *stack;

    /* Predefined server page*/
    GtkComboBox *server_combo_box;

    /* Custom server page*/
    GtkEntry *name_entry;
    GtkEntry *host_entry;
    GtkEntry *port_entry;
    GtkEntry *passwd_entry;
    GtkCheckButton  *tls_check_button;
    GtkCheckButton  *tls_noverify_check_button;

    /* User info */
    GtkEntry *nick_entry;
    GtkEntry *username_entry;
    GtkEntry *realname_entry;

    /* Buttons */
    GtkButton *connect_button;
    GtkButton *cancel_button;

    /* Data model */
    GtkListStore *server_list_store;
};

struct _SrainConnectPopoverClass {
    GtkPopoverClass parent_class;
};

G_DEFINE_TYPE(SrainConnectPopover, srain_connect_popover, GTK_TYPE_POPOVER);

static void srain_connect_popover_init(SrainConnectPopover *self);
static void srain_connect_popover_class_init(SrainConnectPopoverClass *class);
static void server_combo_box_set_model(SrainConnectPopover *dialog);

static void popover_on_visible(GObject *object, GParamSpec *pspec, gpointer data);
static void connect_button_on_click(gpointer user_data);
static void cancel_button_on_click(gpointer user_data);

SrainConnectPopover* srain_connect_popover_new(GtkWidget *relative){
    SrainConnectPopover *popover;

    popover = g_object_new(SRAIN_TYPE_CONNECT_POPOVER, NULL);

    gtk_popover_set_relative_to(GTK_POPOVER(popover), relative);

    return popover;
}
void srain_connect_popover_add_server(SrainConnectPopover *popover, const char *server){
    GtkTreeIter iter;
    GtkListStore *store;

    store = popover->server_list_store;

    if (str_is_empty(server)){
        ERR_FR("Invalid server name: %s", server);
    }

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            SERVER_LIST_STORE_COL_NAME, server,
            -1);
}

void srain_connect_popover_clear(SrainConnectPopover *popover){
    /* Clear data model */
    gtk_list_store_clear(popover->server_list_store);

    /* Clear custom server page input */
    gtk_entry_set_text(popover->name_entry, "");
    gtk_entry_set_text(popover->host_entry, "");
    gtk_entry_set_text(popover->port_entry, "");
    gtk_entry_set_text(popover->passwd_entry, "");
    gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(popover->tls_check_button), FALSE);
    gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(popover->tls_noverify_check_button), FALSE);

    /* Clear user info */
    gtk_entry_set_text(popover->nick_entry, "");
    gtk_entry_set_text(popover->username_entry, "");
    gtk_entry_set_text(popover->realname_entry, "");
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void srain_connect_popover_init(SrainConnectPopover *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    server_combo_box_set_model(self);

    g_signal_connect(self, "notify::visible",
            G_CALLBACK(popover_on_visible), NULL);
    g_signal_connect_swapped(self->connect_button, "clicked",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);
}

static void srain_connect_popover_class_init(SrainConnectPopoverClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/connect_popover.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, stack);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, server_combo_box);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, name_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, host_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, port_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, passwd_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, tls_check_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, tls_noverify_check_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, nick_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, username_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, realname_entry);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, connect_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectPopover, cancel_button);
}

static void server_combo_box_set_model(SrainConnectPopover *dialog){
    GtkListStore *store;

    dialog->server_list_store = gtk_list_store_new(1, G_TYPE_STRING);
    store = dialog->server_list_store;

    gtk_combo_box_set_model(dialog->server_combo_box, GTK_TREE_MODEL(store));
}

static void popover_on_visible(GObject *object, GParamSpec *pspec, gpointer data){
    SrainConnectPopover *popover;

    popover = SRAIN_CONNECT_POPOVER(object);

    if (gtk_widget_get_visible(GTK_WIDGET(popover))){
        /* Update server list while displaying */
        SrnRet ret;
        
        ret = sui_event_hdr(NULL, SUI_EVENT_SERVER_LIST, NULL);
        if (!RET_IS_OK(ret)){
            char *msg;
            msg = g_strdup_printf(_("Failed to get server list: %s"), RET_MSG(ret));
            sui_message_box(_("Error"), msg);
            g_free(msg);
        }
    } else {
        /* Clear all users input while hiding */
        srain_connect_popover_clear(popover);
    }
}

static void connect_button_on_click(gpointer user_data){
    const char *page;
    const char *name;
    const char *host;
    const char *port;
    const char *passwd;
    const char *nick;
    const char *username;
    const char *realname;
    gboolean tls;
    gboolean tls_noverify;
    GVariantDict *params = NULL;
    SrnRet ret;
    SrainConnectPopover *popover;

    popover = user_data;
    page = gtk_stack_get_visible_child_name(popover->stack);
    if (g_strcmp0(page, PAGE_PREDEFINEED_SERVER) == 0){
        GtkTreeIter iter;

        if (gtk_combo_box_get_active_iter(popover->server_combo_box, &iter)){
            gtk_tree_model_get(GTK_TREE_MODEL(popover->server_list_store), &iter,
                    SERVER_LIST_STORE_COL_NAME, &name,
                    -1);
        } else {
            name = "";
        }
        /* The following fields will be ignored */
        host = "";
        port = "";
        passwd = "";
        tls = FALSE;
        tls_noverify = FALSE;
    } else if (g_strcmp0(page, PAGE_CUSTOM_SERVER) == 0){
        name = gtk_entry_get_text(popover->name_entry);
        host = gtk_entry_get_text(popover->host_entry);
        port = gtk_entry_get_text(popover->port_entry);
        passwd = gtk_entry_get_text(popover->passwd_entry);
        tls = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(popover->tls_check_button));
        tls_noverify = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(popover->tls_noverify_check_button));
    } else {
        ERR_FR("Unknown stack page: %s", page);
        return;
    }

    nick = gtk_entry_get_text(popover->nick_entry);
    username = gtk_entry_get_text(popover->username_entry);
    realname = gtk_entry_get_text(popover->realname_entry);

    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "name", SUI_EVENT_PARAM_STRING, name);
    g_variant_dict_insert(params, "host", SUI_EVENT_PARAM_STRING, host);
    g_variant_dict_insert(params, "port", SUI_EVENT_PARAM_INT, atoi(port));
    g_variant_dict_insert(params, "password", SUI_EVENT_PARAM_STRING, passwd);
    g_variant_dict_insert(params, "nick", SUI_EVENT_PARAM_STRING, nick);
    g_variant_dict_insert(params, "username", SUI_EVENT_PARAM_STRING, username);
    g_variant_dict_insert(params, "realname", SUI_EVENT_PARAM_STRING, realname);
    g_variant_dict_insert(params, "tls", SUI_EVENT_PARAM_BOOL, tls);
    g_variant_dict_insert(params, "tls-noverify", SUI_EVENT_PARAM_BOOL, tls_noverify);

    ret = sui_event_hdr(NULL, SUI_EVENT_CONNECT, params);
    g_variant_dict_unref(params);

    if (RET_IS_OK(ret)){
        gtk_widget_set_visible(GTK_WIDGET(popover), FALSE);
    } else {
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void cancel_button_on_click(gpointer user_data){
    SrainConnectPopover *popover;

    popover = user_data;

    gtk_widget_set_visible(GTK_WIDGET(popover), FALSE);
}
