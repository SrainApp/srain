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
#include "path.h"
#include "utils.h"

#include "app_event.h"

/* Only one SrnApplication instance in one application */
static SrnApplication *app_instance = NULL;

static void init_logger(SrnApplication *app);
static void finalize_logger(SrnApplication *app);

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrnApplication* srn_application_new(void){
    char *path;
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
    path = srn_get_system_config_file();
    if (path){
        ret = srn_config_manager_load_system_config(cfg_mgr, path);
        g_free(path);
        if (!RET_IS_OK(ret)){
            sui_message_box(_("Error"), RET_MSG(ret));
        }
    }
    path = srn_get_user_config_file();
    if (path){
        ret = srn_config_manager_load_user_config(cfg_mgr, path);
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

    app->ui = sui_new_application(cfg->id ? cfg->id : PACKAGE_APPID,
            app, &app->ui_app_events, cfg->ui);

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
    GList *lst;
    SrnRet ret;
    SrnLoggerConfig *logger_cfg;
    SrnLoggerConfig *old_logger_cfg;
    SrnApplicationConfig *cfg;
    SrnApplicationConfig *old_cfg;
    SrnConfigManager *cfg_mgr;

    cfg_mgr = app->cfg_mgr;

    /* Read newest user config: TODO: Read should not be done here */
    path = srn_get_user_config_file();
    if (!path){
        return RET_ERR(_("User config not found"));
    }
    ret = srn_config_manager_load_user_config(cfg_mgr, path);
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
    lst = app->srv_list;
    while (lst) {
        SrnServer *srv;
        SrnServerConfig *srv_cfg;
        SrnServerConfig *old_srv_cfg;

        srv = lst->data;
        old_srv_cfg = srv->cfg;
        srv_cfg = srn_server_config_new();

        ret = srn_config_manager_read_server_config(
                app->cfg_mgr, srv_cfg, srv->name);
        if (!RET_IS_OK(ret)){
            goto ERR_RELOAD_SERVER;
        }
        ret = srn_server_config_check(srv_cfg);
        if (!RET_IS_OK(ret)){
            goto ERR_RELOAD_SERVER;
        }
        srn_server_config_free(old_srv_cfg);
        srn_server_set_config(srv, srv_cfg);

        ret = srn_server_reload_config(srv);
        if (!RET_IS_OK(ret)){
            goto ERR_RELOAD_SERVER;
        }

        lst = g_list_next(lst);
        continue;

ERR_RELOAD_SERVER:
        if (srv_cfg != srv->cfg){
            srn_server_config_free(srv_cfg);
        }
        return ret;
    }

    return RET_OK(_("All config reloaded"));

ERR_RELOAD_LOGGER:
    srn_logger_config_free(logger_cfg);
    return RET_ERR(_("Failed to reload logger config: %1$s"),
            RET_MSG(ret));

ERR_RELOAD_APP:
    srn_application_config_free(cfg);
    return RET_ERR(_("Failed to reload application config: %1$s"),
            RET_MSG(ret));

}

SrnRet srn_application_add_server(SrnApplication *app, const char *name){
    SrnRet ret;
    SrnServerConfig *srv_cfg;

    srv_cfg = srn_server_config_new(name);
    ret = srn_config_manager_read_server_config(app->cfg_mgr, srv_cfg, name);
    if (!RET_IS_OK(ret)){
        goto ERR;
    }

    ret = srn_application_add_server_with_config(app, name, srv_cfg);
    if (!RET_IS_OK(ret)){
        goto ERR;
    }

    return SRN_OK;

ERR:
    srn_server_config_free(srv_cfg);
    return ret;
}

SrnRet srn_application_add_server_with_config(SrnApplication *app,
        const char *name, SrnServerConfig *srv_cfg) {
    GList *lst;
    SrnRet ret;
    SrnServer *srv;

    lst = app->srv_list;
    while (lst) {
        srv = lst->data;
        if (g_ascii_strcasecmp(srv->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    ret = srn_server_config_check(srv_cfg);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    srv = srn_server_new(name, srv_cfg);
    app->cur_srv = srv;
    app->srv_list = g_list_append(app->srv_list, srv);

    // Create server chat
    ret = srn_server_add_chat(srv, srv->name);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    /* Auto join chat */
    // FIXME: This should be done in server.c?
    for (GList *lst = srv->cfg->auto_join_chat_list;
            lst;
            lst = g_list_next(lst)){
        const char *name;

        name = lst->data;
        ret = srn_server_add_chat(srv, name);
        if (!RET_IS_OK(ret)){
            ret = RET_ERR(_("Failed to add chat \"%1$s\": %2$s"),
                    name, RET_MSG(ret));
            sui_message_box(_("Error"), RET_MSG(ret));
            continue;
        }
    }

    /* Run server autorun commands */
    for (GList *lst = srv->cfg->auto_run_cmd_list; lst; lst = g_list_next(lst)){
        const char *cmd;
        SrnRet ret;
        SrnChat *chat;

        cmd = lst->data;
        chat = srv->chat;
        ret = srn_chat_run_command(chat, cmd);

        // NOTE: The server and chat may be invlid after running command
        if (!srn_server_is_valid(srv) || !srn_server_is_chat_valid(srv, chat)){
            return ret;
        }

        if (RET_IS_OK(ret)){
            if (ret != SRN_OK) { // Has OK message
                srn_chat_add_misc_message_fmt(chat, chat->_user,
                       _("Server autorun command: %1$s"), RET_MSG(ret));
            }
        } else {
            srn_chat_add_error_message_fmt(chat, chat->_user,
                       _("Server autorun command: %1$s"), RET_MSG(ret));
        }
    }

    return SRN_OK;
}

SrnRet srn_application_rm_server(SrnApplication *app, SrnServer *srv) {
    GList *lst;
    SrnServerConfig *srv_cfg;

    lst = g_list_find(app->srv_list, srv);
    if (!lst){
        return SRN_ERR;
    }
    if (app->cur_srv == srv) {
        app->cur_srv = NULL;
    }
    app->srv_list = g_list_delete_link(app->srv_list, lst);

    srv_cfg = srv->cfg;
    srn_server_free(srv);
    srn_server_config_free(srv_cfg);

    return SRN_OK;
}

SrnServer* srn_application_get_server(SrnApplication *app, const char *name){
    GList *lst;

    lst = app->srv_list;
    while (lst) {
        SrnServer *srv;

        srv = lst->data;
        if (g_ascii_strcasecmp(srv->name, name) == 0){
            return srv;
        }
        lst = g_list_next(lst);
    }

    return NULL;
}

SrnServer* srn_application_get_server_by_addr(SrnApplication *app,
        SrnServerAddr *addr){
    GList *lst;
    SrnRet ret;

    lst = app->srv_list;
    while (lst) {
        GList *addr_lst;
        SrnServer *srv;

        srv = lst->data;
        addr_lst = srv->cfg->addrs;
        while (addr_lst){
            if (srn_server_addr_equal(addr, addr_lst->data)){
                return srv;
            }
            addr_lst = g_list_next(addr_lst);
        }
        lst = g_list_next(lst);
    }

    return NULL;
}

bool srn_application_is_server_valid(SrnApplication *app, SrnServer *srv) {
    return g_list_find(app->srv_list, srv) != NULL;
}

void srn_application_auto_connect_server(SrnApplication *app) {
    SrnRet ret;

    for (GList *lst = app->cfg->auto_connect_srv_list;
            lst;
            lst = g_list_next(lst)){
        const char *name;
        SrnServer *srv;

        name = lst->data;
        ret = srn_application_add_server(app, name);
        if (!RET_IS_OK(ret)){
            ret = RET_ERR(_("Failed to add server \"%1$s\": %2$s"),
                    name, RET_MSG(ret));
            sui_message_box(_("Error"), RET_MSG(ret));
            continue;
        }
        srn_server_connect(srn_application_get_server(app, name));
    }
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

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
