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
#include <libsecret/secret.h>

#include "config/reader.h"
#include "config/password.h"
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

#define LOGIN_PAGE_NONE     "none"
#define LOGIN_PAGE_PASSWORD "password"
#define LOGIN_PAGE_CERT     "certificate"

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
    GtkCheckButton *remember_password_check_button;
    GtkCheckButton *tls_check_button;
    GtkCheckButton *tls_noverify_check_button;

    GtkEntry *nick_entry;
    GtkComboBox *login_method_combo_box;
    GtkStack *login_method_stack;
    GtkEntry *login_password_entry;
    GtkCheckButton *remember_login_password_check_button;
    GtkButton *login_cert_button;


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
static void update_focus(SuiConnectPanel *self);

static void server_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data);
static void login_method_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data);
static void connect_button_on_click(gpointer user_data);
static void cancel_button_on_click(gpointer user_data);
static void nick_entry_on_changed(GtkEditable *editable, gpointer user_data);
static void on_password_lookup(GObject *source, GAsyncResult *result,
        gpointer user_data);
static void stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data);

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

    g_signal_connect(self->stack, "notify::visible-child",
            G_CALLBACK(stack_on_child_changed), self);

    g_signal_connect(self->quick_server_combo_box, "changed",
            G_CALLBACK(server_combo_box_on_changed), self);
    g_signal_connect(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(self->server_combo_box))),
            "changed", G_CALLBACK(server_combo_box_on_changed), self);
    g_signal_connect(self->nick_entry, "changed",
            G_CALLBACK(nick_entry_on_changed), self);
    g_signal_connect(self->login_method_combo_box, "changed",
            G_CALLBACK(login_method_combo_box_on_changed), self);
    g_signal_connect_swapped(self->connect_button, "clicked",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);

    /* Press enter to connect */
    g_signal_connect_swapped(self->quick_nick_entry, "activate",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->host_entry, "activate",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->port_entry, "activate",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->password_entry, "activate",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->nick_entry, "activate",
            G_CALLBACK(connect_button_on_click), self);
    g_signal_connect_swapped(self->login_password_entry, "activate",
            G_CALLBACK(connect_button_on_click), self);

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
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, remember_password_check_button);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, tls_check_button);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, tls_noverify_check_button);

    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, nick_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, login_method_combo_box);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, login_method_stack);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, login_password_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, remember_login_password_check_button);
    gtk_widget_class_bind_template_child(widget_class, SuiConnectPanel, login_cert_button);

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
    if (!srv_name || !strlen(srv_name)){
        gtk_combo_box_set_active_iter(self->server_combo_box, NULL);

        gtk_entry_set_text(self->host_entry, "");
        gtk_entry_set_text(self->port_entry, "");
        gtk_entry_set_text(self->password_entry, "");
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->remember_password_check_button), FALSE);
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->tls_check_button), FALSE);
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->tls_noverify_check_button), FALSE);

        gtk_entry_set_text(self->nick_entry, "");
        gtk_combo_box_set_active_iter(self->login_method_combo_box, NULL);
        gtk_entry_set_text(self->login_password_entry, "");
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(self->remember_login_password_check_button), FALSE);
        gtk_file_chooser_set_filename(
                GTK_FILE_CHOOSER(self->login_cert_button), "");
    } else {
        SrnRet ret;
        SrnApplication *app_model;
        SrnServerConfig *srv_cfg;

        app_model = sui_application_get_ctx(sui_application_get_instance());
        srv_cfg = srn_server_config_new();

        // NOTE: this function blocks UI.
        // pcf said this is good.
        ret = srn_config_manager_read_server_config(
                app_model->cfg_mgr, srv_cfg, srv_name);
        if (!RET_IS_OK(ret)) {
            ERR_FR("Failed to read server config: %1$s", RET_MSG(ret));
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
        if (srv_cfg->password) {
            gtk_entry_set_text(self->password_entry, srv_cfg->password);
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
        gtk_entry_set_text(self->login_password_entry,
                srv_cfg->user->login->password ?
                srv_cfg->user->login->password : "");
        gtk_file_chooser_set_filename(
                GTK_FILE_CHOOSER(self->login_cert_button),
                srv_cfg->user->login->cert_file ?
                srv_cfg->user->login->cert_file : "");

        srn_server_config_free(srv_cfg);
    }
}

static void refresh_server_list(SuiConnectPanel *self){
    GList *lst;
    GList *srv_cfg_lst;
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
        lst = g_list_next(lst);
    }

    g_list_free_full(srv_cfg_lst, g_free);
}

static void refresh_login_method_list(SuiConnectPanel *self){
    GtkTreeIter iter;
    GtkListStore *store;

    // Supported login methods
    static SrnLoginMethod lms[] = {
        SRN_LOGIN_METHOD_NONE,
        SRN_LOGIN_METHOD_NICKSERV,
        SRN_LOGIN_METHOD_MSG_NICKSERV,
        SRN_LOGIN_METHOD_SASL_PLAIN,
        SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE,
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
    GtkEntry *entry;
    SuiConnectPanel *self;

    self = SUI_CONNECT_PANEL(user_data);
    entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(self->server_combo_box)));
    srv_name = gtk_entry_get_text(entry);
    update(self, srv_name);
}

static void login_method_combo_box_on_changed(GtkComboBox *combo_box,
        gpointer user_data){
    const char *page;
    SrnLoginMethod lm;
    GtkTreeModel *model;
    GtkTreeIter iter;
    SuiConnectPanel *self;

    self = SUI_CONNECT_PANEL(user_data);
    model = GTK_TREE_MODEL(self->login_method_list_store);

    if (!gtk_combo_box_get_active_iter(combo_box, &iter)){
        lm = SRN_LOGIN_METHOD_NONE;
    } else {
        // Set the login_method_stack to the corresponding page
        gtk_tree_model_get(model, &iter,
                LOGIN_METHOD_LIST_STORE_COL_ID, &lm,
                -1);
    }

    switch (lm) {
        case SRN_LOGIN_METHOD_NICKSERV:
        case SRN_LOGIN_METHOD_MSG_NICKSERV:
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            page = LOGIN_PAGE_PASSWORD;
            break;
        case SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE:
            page = LOGIN_PAGE_CERT;
            break;
        case SRN_LOGIN_METHOD_NONE:
        default:
            page = LOGIN_PAGE_NONE;
            break;
    }
    gtk_stack_set_visible_child_name(self->login_method_stack, page);
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
            ret = RET_ERR(_("No server selected"));
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
        bool rmb_passwd;
        bool tls;
        bool tls_noverify;
        const char *nick;
        const char *method_str;
        const char *login_passwd;
        bool rmb_login_passwd;
        const char *login_cert_file;

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
        if (!str_is_empty(host)) {
            srn_server_config_clear_addr(srv_cfg);
            srn_server_config_add_addr(srv_cfg, srn_server_addr_new(host, port));
        }

        passwd = gtk_entry_get_text(self->password_entry);
        // Always overwrite password
        str_assign(&srv_cfg->password, passwd);

        rmb_passwd = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(self->remember_password_check_button));
        if (rmb_passwd) {
            if (strlen(passwd)) {  // Reqeust to store password
                ret = srn_config_manager_store_server_password(
                        app_model->cfg_mgr, passwd, srv_name);
                if (!RET_IS_OK(ret)) {
                    ret = RET_ERR(_("Failed to store server password: %1$s"),
                            RET_MSG(ret));
                    sui_message_box(_("Error"), RET_MSG(ret) );
                    // No need to return
                }
            } else {  // Reqeust to clear password
                ret = srn_config_manager_clear_server_password(
                        app_model->cfg_mgr, srv_name);
                if (!RET_IS_OK(ret)) {
                    ret = RET_ERR(_("Failed to clear server password: %1$s"),
                            RET_MSG(ret));
                    sui_message_box(_("Error"), RET_MSG(ret) );
                    // No need to return
                }
            }
        }

        tls_noverify = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(self->tls_noverify_check_button));
        srv_cfg->irc->tls_noverify = tls_noverify;

        tls = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(self->tls_check_button));
        // TODO: Let tls_check_button to be toggled when
        // tls_noverify_check_button is toggled.
        srv_cfg->irc->tls = tls || tls_noverify;

        nick = gtk_entry_get_text(self->nick_entry);
        if (!str_is_empty(nick)) {
            str_assign(&srv_cfg->user->nick, nick);
        }

        method_str = gtk_combo_box_get_active_id(self->login_method_combo_box);
        method = srn_login_method_from_string(method_str);
        srv_cfg->user->login->method = method;

        login_passwd = gtk_entry_get_text(self->login_password_entry);
        // Always overwrite password
        str_assign(&srv_cfg->user->login->password, login_passwd);

        rmb_login_passwd = gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(self->remember_login_password_check_button));
        if (rmb_login_passwd) {
            if (strlen(login_passwd)) { // Reqeust to store password
                ret = srn_config_manager_store_user_password(
                        app_model->cfg_mgr, login_passwd, srv_name, nick);
                if (!RET_IS_OK(ret)) {
                    ret = RET_ERR(_("Failed to store user password: %1$s"),
                            RET_MSG(ret));
                    sui_message_box(_("Error"), RET_MSG(ret) );
                    // No need to return
                }
            } else { // Reqeust to clear password
                ret = srn_config_manager_clear_user_password(
                        app_model->cfg_mgr, srv_name, nick);
                if (!RET_IS_OK(ret)) {
                    ret = RET_ERR(_("Failed to clear user password: 1$%s"),
                            RET_MSG(ret));
                    sui_message_box(_("Error"), RET_MSG(ret) );
                    // No need to return
                }
            }
        }

        login_cert_file = gtk_file_chooser_get_filename(
                GTK_FILE_CHOOSER(self->login_cert_button));
        if (!str_is_empty(login_cert_file)) {
            str_assign(&srv_cfg->user->login->cert_file, login_cert_file);
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
    LOG_FR("Server connect finished");
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


static void nick_entry_on_changed(GtkEditable *editable, gpointer user_data) {
    const char *srv_name;
    const char *user_name;
    GtkEntry *entry;
    SuiConnectPanel *self;
    SrnApplication *app_model;
    SrnConfigManager *cfg_mgr;

    entry = GTK_ENTRY(editable);
    self = SUI_CONNECT_PANEL(user_data);
    app_model = sui_application_get_ctx(sui_application_get_instance());
    cfg_mgr = app_model->cfg_mgr;

    srv_name = gtk_entry_get_text(
            GTK_ENTRY(gtk_bin_get_child(GTK_BIN(self->server_combo_box))));
    user_name = gtk_entry_get_text(entry);

    // Clear login password when user name is not valid
    if (str_is_empty(user_name)) {
        gtk_entry_set_text(self->login_password_entry, "");
        return;
    }

    // Lookup login password asynchronously
    secret_password_lookup(srn_config_manager_get_user_secret_schema(cfg_mgr),
            NULL, on_password_lookup, self->login_password_entry,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_USER, user_name,
            NULL);
}

static void on_password_lookup(GObject *source, GAsyncResult *result,
        gpointer user_data) {
    char *passwd;
    GtkEntry *entry;

    entry = GTK_ENTRY(user_data);

    passwd = secret_password_lookup_finish(result, NULL);
    if (!passwd) {
        return;
    }

    gtk_entry_set_text(entry, passwd);
    secret_password_free(passwd);
}

static void stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data) {
    update_focus(SUI_CONNECT_PANEL(user_data));
}

static void update_focus(SuiConnectPanel *self) {
    const char *page = gtk_stack_get_visible_child_name(self->stack);

    if (g_strcmp0(page, PAGE_QUICK_MODE) == 0){
        gtk_widget_grab_focus(GTK_WIDGET(self->quick_server_combo_box));
    } else if (g_strcmp0(page, PAGE_ADVANCED_MODE) == 0){
        gtk_widget_grab_focus(GTK_WIDGET(self->server_combo_box));
    } else {
        g_warn_if_reached();
    }
}
