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
 * @file srain_connect_dialog.c
 * @brief Connection dialog
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-09-16
 */

#include <stdlib.h>
#include <gtk/gtk.h>

#include "sui/sui.h"
#include "sui_event_hdr.h"
#include "srain_window.h"
#include "srain_connect_dialog.h"

#include "srain.h"
#include "i18n.h"

struct _SrainConnectDialog {
    GtkDialog parent;

    /* Server info */
    GtkEntry *name_entry;
    GtkEntry *host_entry;
    GtkEntry *port_entry;
    GtkEntry *passwd_entry;
    GtkEntry *nick_entry;
    GtkEntry *realname_entry;
    GtkCheckButton  *tls_check_button;
    GtkCheckButton  *tls_noverify_check_button;

    /* Server list */
    GtkListBox *server_list_box;
    GtkButton *add_server_button;
    GtkButton *rm_server_button;

    /* Dialog button */
    GtkButton *cancel_button;
    GtkButton *save_button;
    GtkButton *connect_button;
};

struct _SrainConnectDialogClass {
    GtkDialogClass parent_class;
};

G_DEFINE_TYPE(SrainConnectDialog, srain_connect_dialog, GTK_TYPE_DIALOG);

static void cancel_button_on_click(gpointer user_data);
static void save_button_on_click(gpointer user_data);
static void connect_button_on_click(gpointer user_data);

static void srain_connect_dialog_init(SrainConnectDialog *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);
    g_signal_connect_swapped(self->save_button, "clicked",
            G_CALLBACK(save_button_on_click), self);
    g_signal_connect_swapped(self->connect_button, "clicked",
            G_CALLBACK(connect_button_on_click), self);
}

static void srain_connect_dialog_class_init(SrainConnectDialogClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/connect_dialog.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, name_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, host_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, port_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, passwd_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, nick_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, realname_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, tls_check_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, tls_noverify_check_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, server_list_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, add_server_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, rm_server_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, cancel_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, save_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainConnectDialog, connect_button);
}

SrainConnectDialog* srain_connect_dialog_new(GtkWindow *parent){
    SrainConnectDialog *dialog;

    dialog = g_object_new(SRAIN_TYPE_CONNECT_DIALOG, NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);

    return dialog;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void cancel_button_on_click(gpointer user_data){
    SrainConnectDialog *dialog;

    dialog = user_data;
    gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_CONNECT_DIALOG_RESP_CANCEL);
}

static void save_button_on_click(gpointer user_data){
    SrainConnectDialog *dialog;

    dialog = user_data;
    gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_CONNECT_DIALOG_RESP_SAVE);
}

static void connect_button_on_click(gpointer user_data){
    const char *name;
    const char *host;
    const char *port;
    const char *passwd;
    const char *nick;
    const char *realname;
    gboolean tls;
    gboolean tls_noverify;
    GVariantDict *params;
    SrnRet ret;
    SrainConnectDialog *dialog;

    dialog = user_data;
    params = g_variant_dict_new(NULL);

    name = gtk_entry_get_text(dialog->name_entry);
    host = gtk_entry_get_text(dialog->host_entry);
    port = gtk_entry_get_text(dialog->port_entry);
    passwd = gtk_entry_get_text(dialog->passwd_entry);
    nick = gtk_entry_get_text(dialog->nick_entry);
    realname = gtk_entry_get_text(dialog->realname_entry);
    tls = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->tls_check_button));
    tls_noverify = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->tls_noverify_check_button));

    g_variant_dict_insert(params, "name", SUI_EVENT_PARAM_STRING, name);
    g_variant_dict_insert(params, "host", SUI_EVENT_PARAM_STRING, host);
    g_variant_dict_insert(params, "port", SUI_EVENT_PARAM_INT, atoi(port));
    g_variant_dict_insert(params, "password", SUI_EVENT_PARAM_STRING, passwd);
    g_variant_dict_insert(params, "nick", SUI_EVENT_PARAM_STRING, nick);
    g_variant_dict_insert(params, "realname", SUI_EVENT_PARAM_STRING, realname);
    g_variant_dict_insert(params, "tls", SUI_EVENT_PARAM_BOOL, tls);
    g_variant_dict_insert(params, "tls-noverify", SUI_EVENT_PARAM_BOOL, tls_noverify);

    ret = sui_event_hdr(NULL, SUI_EVENT_CONNECT, params);
    g_variant_dict_unref(params);

    if (RET_IS_OK(ret)){
        gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_CONNECT_DIALOG_RESP_CONNECT);
    } else {
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}
