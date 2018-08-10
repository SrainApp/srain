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
 * @file sui_connect_panel.c
 * @brief Panel widget for connecting to server
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-09-16
 */

#include <stdlib.h>
#include <gtk/gtk.h>

#include "config/reader.h"
#include "sui/sui.h"
#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "utils.h"

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_window.h"
#include "sui_connect_panel.h"

#define SERVER_LIST_STORE_COL_NAME          0
#define LOGIN_METHOD_LIST_STORE_COL_ID      0
#define LOGIN_METHOD_LIST_STORE_COL_NAME    1

#define PAGE_QUICK_MODE     "quick_mode_page"
#define PAGE_ADVANCED_MODE  "advanced_mode_page"

struct _SuiConnectPanel {
    GtkBox parent;

    GtkStack *stack;

    /* Quick mode */
    GtkComboBox *quick_server_combo_box;
    GtkEntry *quick_nick_entry;

    /* Advance mode */
    GtkComboBox *server_combo_box;
    GtkEntry *host_entry;
    GtkEntry *port_entry;
    GtkEntry *password_entry;
    GtkCheckButton *tls_check_button;
    GtkCheckButton *tls_noverify_check_button;

    GtkEntry *nick_entry;
    GtkComboBox *login_method_combo_box;
    GtkStack *login_method_stack;
    GtkEntry *pass_password_entry;
    GtkEntry *nickserv_password_entry;
    GtkEntry *msg_nickserv_password_entry;
    GtkEntry *sasl_plain_identify_entry;
    GtkEntry *sasl_plain_password_entry;

    /* Buttons */
    GtkButton *connect_button;
    GtkButton *cancel_button;

    /* Data model */
    GtkListStore *server_list_store;
    GtkListStore *login_method_list_store;
};

struct _SuiConnectPanelClass {
    GtkBoxClass parent_class;
};

static void update(SuiConnectPanel *self, const char *srv_name);
static void refresh_server_list(SuiConnectPanel *self);
static void refresh_login_method_list(SuiConnectPanel *self);

static void server_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data);
static void login_method_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data);
static void connect_button_on_click(gpointer user_data);
static void cancel_button_on_click(gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiConnectPanel, sui_connect_panel, GTK_TYPE_BOX);

static void sui_connect_panel_init(SuiConnectPanel *self){
    gtk_widget_init_template(GTK_WIDGET(self));

#if GTK_CHECK_VERSION(3, 18, 0)
    gtk_stack_set_interpolate_size(self->stack, TRUE);
    gtk_stack_set_interpolate_size(self->login_method_stack, TRUE);
#endif

    /* Set server list model */
    self->server_list_store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_combo_box_set_model(self->quick_server_combo_box,
            GTK_TREE_MODEL(self->server_list_store));
    gtk_combo_box_set_id_column(self->quick_server_combo_box, 0);
    gtk_combo_box_set_model(self->server_combo_box,
            GTK_TREE_MODEL(self->server_list_store));
    gtk_combo_box_set_id_column(self->server_combo_box, 0);
    gtk_combo_box_set_entry_text_column(self->server_combo_box,
            SERVER_LIST_STORE_COL_NAME);
    g_object_bind_property(
            self->quick_server_combo_box, "active",
            self->server_combo_box, "active",
            G_BINDING_BIDIRECTIONAL);

    self->login_method_list_store = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
    gtk_combo_box_set_model(self->login_method_combo_box,
            GTK_TREE_MODEL(self->login_method_list_store));
    gtk_combo_box_set_id_column(self->login_method_combo_box, 1);

    g_object_bind_property(
            self->quick_nick_entry, "text",
            self->nick_entry, "text",
            G_BINDING_BIDIRECTIONAL);

    g_signal_connect(self->server_combo_box, "changed",
            G_CALLBACK(server_combo_box_on_changed), self);
    g_signal_connect(self->quick_server_combo_box, "changed",
            G_CALLBACK(server_combo_box_on_changed), self);
    g_signal_connect(self->login_method_combo_box, "changed",
            G_CALLBACK(login_method_combo_box_on_changed), self);
    g_signal_connect_swapped(self->connect_button, "clicked",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);

    refresh_server_list(self);
    refresh_login_method_list(self);
}

static void sui_connect_panel_class_init(SuiConnectPanelClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/connect_panel.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, stack);

    /* Quick mode */
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, quick_server_combo_box);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, quick_nick_entry);

    /* Advanced mode */
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, server_combo_box);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, host_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, port_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, password_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, tls_check_button);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, tls_noverify_check_button);

    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, nick_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, login_method_combo_box);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, login_method_stack);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, pass_password_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, nickserv_password_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, msg_nickserv_password_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, sasl_plain_identify_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, sasl_plain_password_entry);

    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, connect_button);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, cancel_button);
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiConnectPanel* sui_connect_panel_new(){
    return g_object_new(SUI_TYPE_CONNECT_PANEL, NULL);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void update(SuiConnectPanel *self, const char *srv_name){
    if (!srv_name){
        gtk_combo_box_set_active_iter(self->server_combo_box, NULL);

        gtk_entry_set_text(self->host_entry, "");
        gtk_entry_set_text(self->port_entry, "");
        gtk_entry_set_text(self->password_entry, "");
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->tls_check_button), FALSE);
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->tls_noverify_check_button), FALSE);

        gtk_entry_set_text(self->nick_entry, "");
        gtk_combo_box_set_active_iter(self->login_method_combo_box, NULL);
        gtk_entry_set_text(self->nick_entry, "");
        gtk_entry_set_text(self->pass_password_entry, "");
        gtk_entry_set_text(self->nickserv_password_entry, "");
        gtk_entry_set_text(self->msg_nickserv_password_entry, "");
        gtk_entry_set_text(self->sasl_plain_identify_entry, "");
        gtk_entry_set_text(self->sasl_plain_password_entry, "");
    } else {
        SrnRet ret;
        SrnApplication *app_model;
        SrnServerConfig *srv_cfg;

        app_model = sui_application_get_ctx(sui_application_get_instance());
        srv_cfg = srn_server_config_new();

        ret = srn_config_manager_read_server_config(
                app_model->cfg_mgr, srv_cfg, srv_name);
        if (!RET_IS_OK(ret)) {
            ERR_FR("Failed to read server config: %s", RET_MSG(ret));
            srn_server_config_free(srv_cfg);
            return;
        }

        if (srv_cfg->addrs){
            char *port;
            SrnServerAddr *addr;

            addr = srv_cfg->addrs->data;
            port = g_strdup_printf("%d", addr->port);
            gtk_entry_set_text(self->host_entry, addr->host);
            gtk_entry_set_text(self->port_entry, port);
            g_free(port);
        }
        if (srv_cfg->passwd) {
            gtk_entry_set_text(self->password_entry, srv_cfg->passwd);
        }
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->tls_check_button),
                srv_cfg->irc->tls);
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->tls_noverify_check_button),
                srv_cfg->irc->tls_noverify);

        gtk_entry_set_text(self->nick_entry,
                srv_cfg->user->nick? srv_cfg->user->nick: "");

        gtk_combo_box_set_active_id(self->login_method_combo_box,
                srn_login_method_to_string(srv_cfg->user->login->method));
        gtk_entry_set_text(self->pass_password_entry,
                srv_cfg->user->login->pass_password ?
                srv_cfg->user->login->pass_password : "");
        gtk_entry_set_text(self->nickserv_password_entry,
                srv_cfg->user->login->nickserv_password ?
                srv_cfg->user->login->nickserv_password : "");
        gtk_entry_set_text(self->msg_nickserv_password_entry,
                srv_cfg->user->login->msg_nickserv_password ?
                srv_cfg->user->login->msg_nickserv_password : "");
        gtk_entry_set_text(self->sasl_plain_identify_entry,
                srv_cfg->user->login->sasl_plain_identify ?
                srv_cfg->user->login->sasl_plain_identify : "");
        gtk_entry_set_text(self->sasl_plain_password_entry,
                srv_cfg->user->login->sasl_plain_password ?
                srv_cfg->user->login->sasl_plain_password : "");

        srn_server_config_free(srv_cfg);
    }
}

static void refresh_server_list(SuiConnectPanel *self){
    GSList *lst;
    GSList *srv_cfg_lst;
    GtkTreeIter iter;
    GtkListStore *store;
    SrnRet ret;
    SrnApplication *app_model;

    app_model = sui_application_get_ctx(sui_application_get_instance());
    store = self->server_list_store;
    gtk_list_store_clear(store);

    srv_cfg_lst = NULL;
    ret = srn_config_manager_read_server_config_list(
            app_model->cfg_mgr, &srv_cfg_lst);
    if (!RET_IS_OK(ret)){
        ERR_FR("%s", RET_MSG(ret));
        return;
    }

    lst = srv_cfg_lst;
    while (lst) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                SERVER_LIST_STORE_COL_NAME, lst->data,
                -1);
        lst = g_slist_next(lst);
    }

    g_slist_free_full(srv_cfg_lst, g_free);
}

static void refresh_login_method_list(SuiConnectPanel *self){
    GtkTreeIter iter;
    GtkListStore *store;

    // Supported login methods
    static SrnLoginMethod lms[] = {
        SRN_LOGIN_METHOD_NONE,
        SRN_LOGIN_METHOD_PASS,
        SRN_LOGIN_METHOD_NICKSERV,
        SRN_LOGIN_METHOD_MSG_NICKSERV,
        SRN_LOGIN_METHOD_SASL_PLAIN,
    };

    store = self->login_method_list_store;
    gtk_list_store_clear(store);

    for (int i = 0; i < sizeof(lms) / sizeof(lms[0]); i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                LOGIN_METHOD_LIST_STORE_COL_ID, (int)lms[i],
                LOGIN_METHOD_LIST_STORE_COL_NAME, srn_login_method_to_string(lms[i]),
                -1);
    }
}

static void server_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data){
    const char *srv_name;
    SuiConnectPanel *self;

    self = SUI_CONNECT_PANEL(user_data);
    srv_name = gtk_combo_box_get_active_id(self->server_combo_box);
    if (srv_name){
        update(self, srv_name);
    }
}

static void login_method_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data){
    SrnLoginMethod lm;
    GtkTreeModel *model;
    GtkTreeIter iter;
    SuiConnectPanel *self;

    self = SUI_CONNECT_PANEL(user_data);
    model = GTK_TREE_MODEL(self->login_method_list_store);

    if (!gtk_combo_box_get_active_iter(combo_box, &iter)){
        lm = SRN_LOGIN_METHOD_NONE;
        return;
    } else {
        // Set the login_method_stack to the corresponding page
        gtk_tree_model_get(model, &iter,
                LOGIN_METHOD_LIST_STORE_COL_ID, &lm,
                -1);
    }

    gtk_stack_set_visible_child_name(self->login_method_stack,
            srn_login_method_to_string(lm));
}

static void connect_button_on_click(gpointer user_data){
    const char *page;
    const char *srv_name;
    SrnRet ret;
    SuiConnectPanel *self;
    SrnApplication *app_model;
    SrnServer *srv;
    SrnServerConfig *srv_cfg;

    ret = SRN_ERR;
    self = user_data;
    page = gtk_stack_get_visible_child_name(self->stack);
    app_model = sui_application_get_ctx(sui_application_get_instance());
    srv = NULL;
    srv_cfg = NULL;

    if (g_ascii_strcasecmp(page, PAGE_QUICK_MODE) == 0){
        const char *nick;

        srv_name = gtk_combo_box_get_active_id(self->quick_server_combo_box);
        if (!srv_name){
            goto FIN;
        }

        srv_cfg = srn_server_config_new();
        ret = srn_config_manager_read_server_config(
                app_model->cfg_mgr, srv_cfg, srv_name);
        if (!RET_IS_OK(ret)){
            goto FIN;
        }

        nick = gtk_entry_get_text(self->quick_nick_entry);

        if (!str_is_empty(nick)) {
            str_assign(&srv_cfg->user->nick, nick);
        }
    } else if (g_ascii_strcasecmp(page, PAGE_ADVANCED_MODE) == 0){
        const char *host;
        int port;
        const char *passwd;
        bool tls;
        bool tls_noverify;
        const char *nick;
        const char *method_str;
        const char *pass_password;
        const char *nickserv_password;
        const char *msg_nickserv_password;
        const char *sasl_plain_identify;
        const char *sasl_plain_password;
        GtkEntry *entry;
        SrnLoginMethod method;

        entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(self->server_combo_box)));
        srv_name = gtk_entry_get_text(entry);

        srv_cfg = srn_server_config_new();
        ret = srn_config_manager_read_server_config(
                app_model->cfg_mgr, srv_cfg, srv_name);
        if (!RET_IS_OK(ret)){
            goto FIN;
        }

        host = gtk_entry_get_text(self->host_entry);
        port = g_ascii_strtoll(gtk_entry_get_text(self->port_entry), NULL, 10);
        passwd = gtk_entry_get_text(self->password_entry);
        tls = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(self->tls_check_button));
        tls_noverify = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(self->tls_noverify_check_button));

        nick = gtk_entry_get_text(self->nick_entry);
        method_str = gtk_combo_box_get_active_id(self->login_method_combo_box);
        method = srn_login_method_from_string(method_str);

        pass_password = gtk_entry_get_text(self->pass_password_entry);
        nickserv_password = gtk_entry_get_text(self->nickserv_password_entry);
        msg_nickserv_password = gtk_entry_get_text(self->msg_nickserv_password_entry);
        sasl_plain_identify = gtk_entry_get_text(self->sasl_plain_identify_entry);
        sasl_plain_password = gtk_entry_get_text(self->sasl_plain_password_entry);

        if (!str_is_empty(host)) {
            srn_server_config_clear_addr(srv_cfg);
            srn_server_config_add_addr(srv_cfg, srn_server_addr_new(host, port));
        }
        if (!str_is_empty(passwd)) {
            str_assign(&srv_cfg->passwd, passwd);
        }
        srv_cfg->irc->tls = tls || tls_noverify;
        srv_cfg->irc->tls_noverify = tls_noverify;

        if (!str_is_empty(nick)) {
            str_assign(&srv_cfg->user->nick, nick);
        }
        srv_cfg->user->login->method = method;

        if (!str_is_empty(pass_password)) {
            str_assign(&srv_cfg->user->login->pass_password, pass_password);
        }
        if (!str_is_empty(nickserv_password)) {
            str_assign(&srv_cfg->user->login->nickserv_password,
                    nickserv_password);
        }
        if (!str_is_empty(msg_nickserv_password)) {
            str_assign(&srv_cfg->user->login->msg_nickserv_password,
                    msg_nickserv_password);
        }
        if (!str_is_empty(sasl_plain_identify)) {
            str_assign(&srv_cfg->user->login->sasl_plain_identify, sasl_plain_identify);
        }
        if (!str_is_empty(sasl_plain_password)) {
            str_assign(&srv_cfg->user->login->sasl_plain_password, sasl_plain_password);
        }
    } else {
        g_warn_if_reached();
        goto FIN;
    }

    ret = srn_application_add_server_with_config(app_model, srv_name, srv_cfg);
    if (!RET_IS_OK(ret)){
        goto FIN;
    }
    srv_cfg = NULL; // Ownership changed to server

    srv = srn_application_get_server(app_model, srv_name);
    if (!srn_server_is_valid(srv)){
        g_warn_if_reached();
        goto FIN;
    }

    ret = srn_server_connect(srv);
    if (!RET_IS_OK(ret)){
        goto FIN;
    }

    ret = SRN_OK;
FIN:
    if (srv_cfg){
        srn_server_config_free(srv_cfg);
    }
    if (RET_IS_OK(ret)){
        sui_common_popdown_panel(GTK_WIDGET(self));
        update(self, NULL);
    } else {
        if (srv){
            srn_application_rm_server(app_model, srv);
        }
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void cancel_button_on_click(gpointer user_data){
    SuiConnectPanel *self;

    self = user_data;

    sui_common_popdown_panel(GTK_WIDGET(self));
    update(self, NULL);
}
