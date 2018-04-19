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
#include "meta.h"
#include "log.h"
#include "i18n.h"
#include "file_helper.h"
#include "rc.h"
#include "utils.h"

#include "app_event.h"

/* Only one SrnApplication instance in one application */
static SrnApplication *app_instance = NULL;

static void init_logger(SrnApplication *app);
static void finalize_logger(SrnApplication *app);
static SrnRet add_server_config(SrnApplication *app, SrnServerConfig *srv_cfg);
static SrnRet steal_server_config(SrnApplication *app, SrnServerConfig *srv_cfg);

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

    init_logger(app);
    srn_application_init_ui_event(app);
    srn_application_init_irc_event(app);

    // Init server config list
    srv_cfg_list = NULL;
    srn_config_manager_read_server_config_list(cfg_mgr, &srv_cfg_list);
    for (GSList *lst = srv_cfg_list; lst; lst = g_slist_next(lst)) {
        SrnServerConfig *srv_cfg;

        ret = srn_application_add_server_config(app, lst->data);
        if (!RET_IS_OK(ret)) {
            sui_message_box(_("Error"), RET_MSG(ret));
        }
        srv_cfg = srn_application_get_server_config(app, lst->data);
        srv_cfg->predefined = TRUE;
    }
    g_slist_free_full(srv_cfg_list, g_free);

    app->ui = sui_new_application(cfg->id, app, &app->ui_app_events, cfg->ui);

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
    // TODO
}

void srn_application_run(SrnApplication *app, int argc, char *argv[]){
    sui_run_application(app->ui, argc, argv);
}

void srn_application_set_config(SrnApplication *app, SrnApplicationConfig  *cfg){
    sui_application_set_config(app->ui, cfg->ui);
    app->cfg = cfg;
}

SrnRet srn_application_reload_config(SrnApplication *app){
    char *path;
    GSList *lst;
    GSList *srv_cfg_list;
    SrnRet ret;
    SrnLoggerConfig *logger_cfg;
    SrnLoggerConfig *old_logger_cfg;
    SrnApplicationConfig *cfg;
    SrnApplicationConfig *old_cfg;
    SrnConfigManager *cfg_mgr;

    cfg_mgr = app->cfg_mgr;

    /* Read newest user config */
    path = get_config_file("srain.cfg");
    if (!path){
        return RET_ERR(_("User config not found"));
    }
    ret = srn_config_manager_read_user_config(cfg_mgr, path);
    g_free(path);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    /* Update log config */
    logger_cfg = srn_logger_config_new();
    old_logger_cfg = srn_logger_get_config(app->logger);
    ret = srn_config_manager_read_log_config(cfg_mgr, logger_cfg);
    if (!RET_IS_OK(ret)){
        goto ERR_RELOAD_LOGGER;
    }
    ret = srn_logger_config_check(logger_cfg);
    if (!RET_IS_OK(ret)){
        goto ERR_RELOAD_LOGGER;
    }
    srn_logger_set_config(app->logger, logger_cfg);
    srn_logger_config_free(old_logger_cfg);

    /* Update application config */
    old_cfg = app->cfg;
    cfg = srn_application_config_new();
    ret = srn_config_manager_read_application_config(cfg_mgr, cfg);
    if (!RET_IS_OK(ret)){
        goto ERR_RELOAD_APP;
    }
    ret = srn_application_config_check(cfg);
    if (!RET_IS_OK(ret)){
        goto ERR_RELOAD_APP;
    }
    srn_application_set_config(app, cfg);
    srn_application_config_free(old_cfg);

    /* Update server configs */
    ret = srn_config_manager_read_server_config_list(cfg_mgr, &srv_cfg_list);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Failed to reload server config list: %1$s"),
                RET_MSG(ret));
    }
    lst = srv_cfg_list;
    while (lst) {
        const char *name;
        SrnServer *srv;
        SrnServerConfig *srv_cfg;
        SrnServerConfig *old_srv_cfg;

        name = lst->data;
        srv = NULL;

        old_srv_cfg = srn_application_get_server_config(app, name);
        if (old_srv_cfg){
            srv = old_srv_cfg->srv;
            steal_server_config(app, old_srv_cfg);
        }

        ret = srn_application_add_server_config(app, name);
        if (!RET_IS_OK(ret)){
            add_server_config(app, old_srv_cfg);
            goto ERR_RELOAD_SERVER;
        }

        srv_cfg = srn_application_get_server_config(app, name);
        srv_cfg->predefined = TRUE;
        ret = srn_server_config_check(srv_cfg);
        if (!RET_IS_OK(ret)){
            srn_application_rm_server_config(app, srv_cfg);
            add_server_config(app, old_srv_cfg);
            goto ERR_RELOAD_SERVER;
        }

        if (old_srv_cfg){
            old_srv_cfg->srv = NULL;
            srn_server_config_free(old_srv_cfg);
        }

        if (srv){
            srn_server_set_config(srv, srv_cfg);
            srv_cfg->srv = srv;
            ret = srn_server_reload_config(srv);
            if (!RET_IS_OK(ret)){
                goto ERR_RELOAD_SERVER;
            }
        }
        lst = g_slist_next(lst);
    }
    g_slist_free_full(srv_cfg_list, g_free);

    return RET_OK(_("All config reloaded"));

ERR_RELOAD_LOGGER:
    srn_logger_config_free(logger_cfg);
    return RET_ERR(_("Failed to reload logger config: %1$s"),
            RET_MSG(ret));

ERR_RELOAD_APP:
    srn_application_config_free(cfg);
    return RET_ERR(_("Failed to reload application config: %1$s"),
            RET_MSG(ret));

ERR_RELOAD_SERVER:
    g_slist_free_full(srv_cfg_list, g_free);
    return ret;
}

SrnRet srn_application_add_server(SrnApplication *app, SrnServerConfig *srv_cfg) {
    SrnRet ret;
    SrnServer *srv;

    g_return_val_if_fail(srn_application_is_server_config_valid(app, srv_cfg), SRN_ERR);
    g_return_val_if_fail(!srv_cfg->srv, RET_ERR(_("Server already exists")));

    ret = srn_server_config_check(srv_cfg);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    srv = srn_server_new(srv_cfg);
    srv_cfg->srv = srv; // Link server to its cfg
    app->cur_srv = srv;
    app->srv_list = g_slist_append(app->srv_list, srv);

    // Create server chat
    ret = srn_server_add_chat(srv, srv->cfg->name);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    return SRN_OK;
}

SrnRet srn_application_rm_server(SrnApplication *app, SrnServer *srv) {
    GSList *lst;
    SrnServerConfig *srv_cfg;

    lst = g_slist_find(app->srv_list, srv);
    if (!lst){
        return SRN_ERR;
    }
    if (app->cur_srv == srv) {
        app->cur_srv = NULL;
    }
    app->srv_list = g_slist_delete_link(app->srv_list, lst);

    srv_cfg = srv->cfg;
    srv_cfg->srv = NULL; // SrnServerConfig is held by SrnApplication, just unlink it
    srn_server_free(srv);

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

SrnRet srn_application_add_server_config(SrnApplication *app, const char *name){
    SrnRet ret;
    SrnServerConfig *srv_cfg = NULL;

    g_return_val_if_fail(!str_is_empty(name), SRN_ERR);

    srv_cfg = srn_server_config_new(name);
    ret = srn_config_manager_read_server_config(
            app->cfg_mgr, srv_cfg, srv_cfg->name);
    if (!RET_IS_OK(ret)){
        goto ERR;
    }

    ret = add_server_config(app, srv_cfg);
    if (!RET_IS_OK(ret)){
        goto ERR;
    }

    return SRN_OK;
ERR:
    if (srv_cfg) {
        srn_server_config_free(srv_cfg);
    }
    return ret;
}

SrnServerConfig* srn_application_add_and_get_server_config_from_basename(
        SrnApplication *app, const char *base){
    int retry;
    char *name;
    SrnServerConfig *srv_cfg;

    retry = 0;
    name = g_strdup(base);
    srv_cfg = NULL;
    do {
        SrnRet ret;

        ret = srn_application_add_server_config(app, name);
        if (RET_IS_OK(ret)){
            srv_cfg = srn_application_get_server_config(app, name);
            break;
        }
        g_free(name);
        retry++;
        name = g_strdup_printf("%s#%d", base, retry);
    } while (retry < 10);
    g_free(name);

    return srv_cfg;
}

SrnRet srn_application_rm_server_config(SrnApplication *app,
        SrnServerConfig *srv_cfg){
    SrnRet ret;

    if (srv_cfg->predefined) {
        return SRN_ERR;
    }
    if (srv_cfg->srv) {
        return SRN_ERR;
    }

    ret = steal_server_config(app, srv_cfg);
    if (!RET_IS_OK(ret)){
        return ret;
    }
    srn_server_config_free(srv_cfg);

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

SrnServerConfig* srn_application_get_server_config_by_host_port(
        SrnApplication *app, const char *host, int port){
    GSList *lst;

    lst = app->srv_cfg_list;
    while (lst) {
        GSList *addr_lst;
        SrnServerConfig *srv_cfg;

        srv_cfg = lst->data;
        addr_lst = srv_cfg->addrs;
        while (addr_lst) {
            SrnServerAddr *addr;

            addr = addr_lst->data;
            // Some urls may not contain port
            if (g_ascii_strcasecmp(addr->host, host) == 0
                    && (addr->port == port || port == 0)){
                return srv_cfg;
            }
            addr_lst = g_slist_next(addr_lst);
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

bool srn_application_is_server_config_valid(SrnApplication *app,
        SrnServerConfig *srv_cfg) {
    return g_slist_find(app->srv_cfg_list, srv_cfg) != NULL;
}

static void init_logger(SrnApplication *app) {
    SrnRet ret;

    app->logger_cfg = srn_logger_config_new();
    ret = srn_config_manager_read_log_config(app->cfg_mgr, app->logger_cfg);
    if (!RET_IS_OK(ret)) {
        // TODO
    }
    app->logger = srn_logger_new(app->logger_cfg);
    srn_logger_set_default(app->logger);
}

static void finalize_logger(SrnApplication *app) {
    srn_logger_free(app->logger);
    srn_logger_config_free(app->logger_cfg);
}

static SrnRet add_server_config(SrnApplication *app, SrnServerConfig *srv_cfg){
    GSList *lst;

    lst = app->srv_cfg_list;
    while (lst){
        SrnServerConfig *old_srv_cfg;

        old_srv_cfg = lst->data;
        if (g_ascii_strcasecmp(old_srv_cfg->name, srv_cfg->name) == 0){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }
    app->srv_cfg_list = g_slist_append(app->srv_cfg_list, srv_cfg);

    return SRN_OK;
}

/**
 * @brief Steal the ownership of srv_cfg from SrnApplication's server config
 *      list.
 *
 * @param app
 * @param srv_cfg
 *
 * @return Returns SRN_OK while successed.
 */
static SrnRet steal_server_config(SrnApplication *app, SrnServerConfig *srv_cfg){
    GSList *lst;

    lst = g_slist_find(app->srv_cfg_list, srv_cfg);
    if (!lst){
        return SRN_ERR;
    }
    app->srv_cfg_list = g_slist_delete_link(app->srv_cfg_list, lst);

    return SRN_OK;
}
