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
 * @file app.c
 * @brief Srain's application class implementation
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include "core/core.h"
#include "sui/sui.h"
#include "config/reader.h"

#include "filter.h"
#include "decorator.h"
#include "server_ui_event.h"
#include "server_irc_event.h"
#include "meta.h"
#include "log.h"
#include "i18n.h"
#include "file_helper.h"
#include "rc.h"

/* Only one SrnApplication instance in one application */
static SrnApplication *app_instance = NULL;

static void srn_application_init_event(SrnApplication *app);
static void srn_application_init_logger(SrnApplication *app);
static void srn_application_finalize_logger(SrnApplication *app);

SrnApplication* srn_application_new(void){
    char *path;
    GSList *srv_cfg_list;
    SrnRet ret;
    SrnVersion *ver;
    SrnConfigManager *cfg_mgr;
    SrnApplication *app;
    SrnApplicationConfig *cfg;

    // Keep only one instance
    g_return_val_if_fail(!app_instance, NULL);

    ver = srn_version_new(PACKAGE_VERSION PACKAGE_BUILD);
    ret = srn_version_parse(ver);
    if (!RET_IS_OK(ret)){
        ERR_FR("Failed to parse " PACKAGE_VERSION PACKAGE_BUILD
                "as application version: %s", RET_MSG(ret));
        return NULL;
    }

    // Init config
    cfg_mgr = srn_config_manager_new(ver);
    path = get_system_config_file("builtin.cfg");
    if (path){
        ret = srn_config_manager_read_system_config(cfg_mgr, path);
        g_free(path);
        if (!RET_IS_OK(ret)){
            sui_message_box(_("Error"), RET_MSG(ret));
        }
    }
    path = get_config_file("srain.cfg");
    if (path){
        ret = srn_config_manager_read_user_config(cfg_mgr, path);
        g_free(path);
        if (!RET_IS_OK(ret)){
            sui_message_box(_("Error"), RET_MSG(ret));
        }
    }
    cfg = srn_application_config_new();
    srn_config_manager_read_application_config(cfg_mgr, cfg);

    app = g_malloc0(sizeof(SrnApplication));
    app->ver = ver;
    app->cfg = cfg;
    app->cfg_mgr = cfg_mgr;

    srn_application_init_event(app);
    srn_application_init_logger(app);

    // Init server config list
    srv_cfg_list = NULL;
    srn_config_manager_read_server_config_list(cfg_mgr, &srv_cfg_list);
    for (GSList *lst = srv_cfg_list; lst; lst = g_slist_next(lst)) {
        srn_application_add_server_config(app, lst->data);
    }
    g_slist_free(srv_cfg_list);

    app->ui = sui_new_application(cfg->id, &app->ui_app_events, cfg->ui);
    sui_application_set_ctx(app->ui, app);

    filter_init(); // FIXME
    decorator_init();
    app_instance = app;

    return app;
}

// SrnApplication* srn_application_get_instance(void){
SrnApplication* srn_application_get_default(void){
    return app_instance;
}

void srn_application_quit(SrnApplication *app){
    g_application_quit(G_APPLICATION(app));
}

void srn_application_run(SrnApplication *app, int argc, char *argv[]){
    sui_run_application(app->ui, argc, argv);
}

SrnServer* srn_application_add_server(SrnApplication *app, const char *name) {
    SrnRet ret;
    SrnServer *srv;
    SrnServerConfig *cfg;

    cfg = srn_application_get_server_config(app, name);
    g_return_val_if_fail(cfg, NULL);
    g_return_val_if_fail(!cfg->srv, NULL);
    g_return_val_if_fail(RET_IS_OK(srn_server_config_check(cfg)), NULL);

    srv = srn_server_new(cfg);
    g_return_val_if_fail(srv, NULL);

    cfg->srv = srv;
    app->srv_list = g_slist_append(app->srv_list, srv);

    ret = srn_server_add_chat(srv, META_SERVER);
    g_return_val_if_fail(RET_IS_OK(ret), NULL);

    return srv;
}

SrnRet srn_application_rm_server(SrnApplication *app, SrnServer *srv) {
    GSList *lst;
    SrnServerConfig *srv_cfg;

    lst = g_slist_find(app->srv_list, srv);
    if (!lst){
        return SRN_ERR;
    }
    app->srv_list = g_slist_delete_link(app->srv_list, lst);

    srv_cfg = srv->cfg;
    srn_server_free(srv);
    srn_application_rm_server_config(app, srv_cfg);

    return SRN_OK;
}

SrnServer* srn_application_get_server(SrnApplication *app, const char *name){
    GSList *lst;

    lst = app->srv_list;
    while (lst) {
        SrnServer *srv;

        srv = lst->data;
        if (g_ascii_strcasecmp(srv->cfg->name, name) == 0){
            return srv;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}

bool srn_application_is_server_valid(SrnApplication *app, SrnServer *srv) {
    return g_slist_find(app->srv_list, srv) != NULL;
}

SrnRet srn_application_add_server_config(SrnApplication *app,
        SrnServerConfig *srv_cfg){
    GSList *lst;
    SrnServerConfig *srv_cfg2;

    lst = app->srv_list;
    while (lst){
        srv_cfg2 = lst->data;
        if (g_ascii_strcasecmp(srv_cfg->name, srv_cfg2->name) == 0){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }
    app->srv_cfg_list = g_slist_append(app->srv_cfg_list, srv_cfg);

    return SRN_OK;
}

SrnRet srn_application_rm_server_config(SrnApplication *app,
        SrnServerConfig *srv_cfg){
    GSList *lst;

    g_return_val_if_fail(!srv_cfg, SRN_ERR);
    if (srv_cfg->predefined) {
        return SRN_OK;
    }

    lst = g_slist_find(app->srv_cfg_list, srv_cfg);
    if (!lst){
        return SRN_ERR;
    }
    app->srv_cfg_list = g_slist_delete_link(app->srv_cfg_list, lst);

    return SRN_OK;
}

SrnServerConfig* srn_application_get_server_config(SrnApplication *app,
        const char *name) {
    GSList *lst;
    SrnServerConfig *srv_cfg;

    lst = app->srv_cfg_list;
    while (lst) {
        srv_cfg = lst->data;
        if (g_ascii_strcasecmp(srv_cfg->name, name) == 0) {
            return srv_cfg;
        }
        lst = g_slist_next(lst);
    }
    return NULL;
}

char* srn_application_dump_server_config_list(SrnApplication *app){
    char *dump;
    GSList *lst;
    GString *str;

    str = g_string_new("");

    lst = app->srv_cfg_list;
    while (lst){
        char *srv_dump;
        SrnServerConfig *srv_cfg;

        srv_cfg = lst->data;
        srv_dump = srn_server_config_dump(srv_cfg);
        str = g_string_append(str, srv_dump);
        g_free(srv_dump);

        lst = g_slist_next(lst);
        if (lst) str = g_string_append(str, "\n\n");
    }

    dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

static void srn_application_init_event(SrnApplication *app) {
    /* UI event */
    app->ui_app_events.open = srn_server_ui_event_open;
    app->ui_app_events.activate = srn_server_ui_event_activate;
    app->ui_app_events.shutdown = srn_server_ui_event_shutdown;

    app->ui_win_events.connect = srn_server_ui_event_connect;
    app->ui_win_events.server_list = srn_server_ui_event_server_list;

    app->ui_events.disconnect = srn_server_ui_event_disconnect;
    app->ui_events.quit = srn_server_ui_event_quit;
    app->ui_events.send = srn_server_ui_event_send;
    app->ui_events.join = srn_server_ui_event_join;
    app->ui_events.part = srn_server_ui_event_part;
    app->ui_events.query = srn_server_ui_event_query;
    app->ui_events.unquery = srn_server_ui_event_unquery;
    app->ui_events.kick = srn_server_ui_event_kick;
    app->ui_events.invite = srn_server_ui_event_invite;
    app->ui_events.whois = srn_server_ui_event_whois;
    app->ui_events.ignore = srn_server_ui_event_ignore;
    app->ui_events.cutover = srn_server_ui_event_cutover;
    app->ui_events.chan_list = srn_server_ui_event_chan_list;

    /* IRC event */
    app->irc_events.connect = srn_server_irc_event_connect;
    app->irc_events.connect_fail = srn_server_irc_event_connect_fail;
    app->irc_events.disconnect = srn_server_irc_event_disconnect;
    app->irc_events.welcome = srn_server_irc_event_welcome;
    app->irc_events.nick = srn_server_irc_event_nick;
    app->irc_events.quit = srn_server_irc_event_quit;
    app->irc_events.join = srn_server_irc_event_join;
    app->irc_events.part = srn_server_irc_event_part;
    app->irc_events.mode = srn_server_irc_event_mode;
    app->irc_events.umode = srn_server_irc_event_umode;
    app->irc_events.topic = srn_server_irc_event_topic;
    app->irc_events.kick = srn_server_irc_event_kick;
    app->irc_events.channel = srn_server_irc_event_channel;
    app->irc_events.privmsg = srn_server_irc_event_privmsg;
    app->irc_events.notice = srn_server_irc_event_notice;
    app->irc_events.channel_notice = srn_server_irc_event_channel_notice;
    app->irc_events.invite = srn_server_irc_event_invite;
    app->irc_events.ctcp_req = srn_server_irc_event_ctcp_req;
    app->irc_events.ctcp_rsp = srn_server_irc_event_ctcp_rsp;
    app->irc_events.cap = srn_server_irc_event_cap;
    app->irc_events.authenticate = srn_server_irc_event_authenticate;
    app->irc_events.ping = srn_server_irc_event_ping;
    app->irc_events.pong = srn_server_irc_event_pong;
    app->irc_events.error = srn_server_irc_event_error;
    app->irc_events.numeric = srn_server_irc_event_numeric;
}

static void srn_application_init_logger(SrnApplication *app) {
    SrnRet ret;

    app->logger_cfg = srn_logger_config_new();
    ret = srn_config_manager_read_log_config(app->cfg_mgr, app->logger_cfg);
    if (!RET_IS_OK(ret)) {
        // TODO
    }
    app->logger = srn_logger_new(app->logger_cfg);
    srn_logger_set_default(app->logger);
}

static void srn_application_finalize_logger(SrnApplication *app) {
    srn_logger_free(app->logger);
    srn_logger_config_free(app->logger_cfg);
}
