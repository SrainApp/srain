/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
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
 * @file reader.c
 * @brief Configuration readers for various config structures
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.3
 * @date 2018-02-16
 */

#include <glib.h>

#include "core/core.h"
#include "config/config.h"
#include "config/password.h"
#include "i18n.h"
#include "utils.h"
#include "log.h"

/* Helpers */
static void settings_get_string_ex(GSettings *settings, const char* key, char **value);
static void settings_get_boolean_ex(GSettings *settings, const char* key, gboolean *value);
static void settings_get_strv_ex(GSettings *settings, const char* key, gchar ***value);

/* Configuration readers for various config structures */
static SrnRet read_log_config_from_cfg(SrnSettingsStruct *cfg, SrnLoggerConfig *log_cfg);
static SrnRet read_log_targets_from_log(GSettings *log, const char *name, GList **lst);

static SrnRet read_application_config_from_cfg(SrnSettingsStruct *cfg, SrnApplicationConfig *app_cfg);

static SrnRet read_server_config_list_from_cfg(SrnSettingsStruct *cfg, GList **srv_cfg_list);
static SrnRet read_server_config_from_server(GSettings *server, SrnServerConfig *cfg);
static SrnRet read_server_config_from_server_list(SrnSettingsStruct *gs, SrnServerConfig *cfg, const char *srv_name);
static SrnRet read_server_config_from_cfg(SrnSettingsStruct *cfg, SrnServerConfig *srv_cfg, const char *srv_name);

static SrnRet read_chat_config_from_chat(GSettings *chat, SrnChatConfig *cfg);
static SrnRet read_chat_config_from_chat_list(SrnSettingsStruct *gs, gchar **chat_list, SrnChatConfig *cfg, const char *chat_name);
static SrnRet read_chat_config_from_cfg(SrnSettingsStruct *cfg, SrnChatConfig *chat_cfg, const char *srv_name, const char *chat_name);

static SrnRet read_user_config_from_server(GSettings *server, SrnUserConfig *cfg);

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrnRet srn_config_manager_read_log_config(SrnConfigManager *mgr,
        SrnLoggerConfig *cfg){
    SrnRet ret;

    ret = read_log_config_from_cfg(&mgr->gs_system_cfg, cfg);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading log config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }*/
    ret = read_log_config_from_cfg(&mgr->gs_user_cfg, cfg);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading log config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }*/

    return SRN_OK;
}

SrnRet srn_config_manager_read_application_config(SrnConfigManager *mgr,
        SrnApplicationConfig *cfg){
    SrnRet ret;

    ret = read_application_config_from_cfg(&mgr->gs_system_cfg, cfg);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading application config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }*/
    ret = read_application_config_from_cfg(&mgr->gs_user_cfg, cfg);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading application config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }*/

    return SRN_OK;
}

SrnRet srn_config_manager_read_server_config_list(SrnConfigManager *mgr,
        GList **srv_cfg_list){
    SrnRet ret;

    ret = read_server_config_list_from_cfg(&mgr->gs_system_cfg, srv_cfg_list);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading server list config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }*/
    ret = read_server_config_list_from_cfg(&mgr->gs_user_cfg, srv_cfg_list);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading server list config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }*/

    return SRN_OK;
}

SrnRet srn_config_manager_read_server_config(SrnConfigManager *mgr,
        SrnServerConfig *cfg, const char *srv_name){
    SrnRet ret;

    ret = read_server_config_from_cfg(&mgr->gs_system_cfg, cfg, srv_name);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading server config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }*/
    ret = read_server_config_from_cfg(&mgr->gs_user_cfg, cfg, srv_name);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading server config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }*/
    ret = srn_config_manager_lookup_server_password(mgr, &cfg->password, srv_name);
    if (!RET_IS_OK(ret)){
        WARN_FR(_("Error occurred while looking up server password: %1$s"),
                RET_MSG(ret));
    }
    ret = srn_config_manager_lookup_user_password(mgr,
            &cfg->user->login->password, srv_name, cfg->user->nick);
    if (!RET_IS_OK(ret)){
        WARN_FR(_("Error occurred while looking up user password: %1$s"),
                RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet srn_config_manager_read_server_config_by_addr(SrnConfigManager *mgr,
        SrnServerConfig *cfg, SrnServerAddr *addr){
    GList *lst;
    GList *cfg_lst;
    SrnRet ret;
    SrnServerConfig *tmp;

    cfg_lst = NULL;
    ret = srn_config_manager_read_server_config_list(mgr, &cfg_lst);
    tmp = NULL;
    if (!RET_IS_OK(ret)){
        goto FIN;
    }

    lst = cfg_lst;
    while (lst) {
        GList *addr_lst;

        tmp = srn_server_config_new();
        // Do not read config to cfg until the address is found
        ret = srn_config_manager_read_server_config(mgr, tmp, lst->data);
        if (!RET_IS_OK(ret)){
            goto FIN;
        }

        addr_lst = tmp->addrs;
        while (addr_lst){
            if (srn_server_addr_equal(addr, addr_lst->data)){
                ret = srn_config_manager_read_server_config(mgr, cfg, lst->data);
                goto FIN;
            }
            addr_lst = g_list_next(addr_lst);
        }

        srn_server_config_free(tmp);
        tmp = NULL;
        lst = g_list_next(lst);
    }

    ret = SRN_ERR;
FIN:
    if (cfg_lst) {
        g_list_free_full(cfg_lst, g_free);
    }
    if (tmp) {
        srn_server_config_free(tmp);
    }
    return ret;
}

SrnRet srn_config_manager_read_chat_config(SrnConfigManager *mgr,
        SrnChatConfig *cfg, const char *srv_name, const char *chat_name){
    SrnRet ret;

    ret = read_chat_config_from_cfg(&mgr->gs_system_cfg, cfg, srv_name, chat_name);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading chat config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }*/

    ret = read_chat_config_from_cfg(&mgr->gs_user_cfg, cfg, srv_name, chat_name);
    /*if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading chat config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }*/

    // In fact we don't known whether this chat is channel
    ret = srn_config_manager_lookup_channel_password(mgr,
            &cfg->password, srv_name, chat_name);
    if (!RET_IS_OK(ret)){
        WARN_FR(_("Error occurred while looking up channel password: %1$s"),
                RET_MSG(ret));
    }

    return SRN_OK;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static SrnRet read_log_config_from_cfg(SrnSettingsStruct *cfg, SrnLoggerConfig *log_cfg){
    SrnRet ret;
    GSettings *log;

    log = g_settings_new_with_backend(PACKAGE_GSCHEMA_LOGGING_ID, cfg->gs_backend);
    if (log){
        log_cfg->prompt_color = g_settings_get_boolean(log, "prompt-color");
        log_cfg->prompt_file = g_settings_get_boolean(log, "prompt-file");
        log_cfg->prompt_function = g_settings_get_boolean(log, "prompt-function");
        log_cfg->prompt_line = g_settings_get_boolean(log, "prompt-line");

        ret = read_log_targets_from_log(log, "debug-targets", &log_cfg->debug_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "info-targets", &log_cfg->info_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "warn-targets", &log_cfg->warn_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "error-targets", &log_cfg->error_targets);
        if (!RET_IS_OK(ret)) return ret;
    }

    g_object_unref(log);
    return SRN_OK;
}

static SrnRet read_log_targets_from_log(GSettings *log, const char *name,
        GList **lst){
    gchar **targets;

    targets = g_settings_get_strv(log, name);
    //if (!targets) return SRN_OK;

    guint count = g_strv_length(targets);
    for (guint i = 0; i < count; i++){
        const char *val;

        val = targets[i];
        if (!val) continue;

        *lst = g_list_append(*lst, g_strdup(val));
    }

    g_strfreev(targets);
    return SRN_OK;
}
static SrnRet read_application_config_from_cfg(SrnSettingsStruct *cfg,
        SrnApplicationConfig *app_cfg){
    //app_cfg->id = g_settings_get_string(cfg->gs_cfg, "id");
    app_cfg->id = NULL;
    app_cfg->ui->theme = g_settings_get_string(cfg->gs_cfg, "theme");
    app_cfg->ui->window.csd = g_settings_get_boolean(cfg->gs_cfg, "csd");
    app_cfg->ui->window.send_on_ctrl_enter = g_settings_get_boolean(cfg->gs_cfg,
                                                                    "send-on-ctrl-enter");
    app_cfg->ui->window.exit_on_close = g_settings_get_boolean(cfg->gs_cfg, "exit-on-close");

    /* Read auto connect server list */
    gchar **auto_connect;
    auto_connect = g_settings_get_strv(cfg->gs_cfg, "auto-connect");
    if (auto_connect){
        for (int i = 0; i < g_strv_length(auto_connect); i++){
            const char *val;

            val = auto_connect[i];
            if (!val) continue;

            app_cfg->auto_connect_srv_list = g_list_append(
                    app_cfg->auto_connect_srv_list, g_strdup(val));
        }
    }

    g_strfreev(auto_connect);
    return SRN_OK;
}

static SrnRet read_server_config_list_from_cfg(SrnSettingsStruct *cfg,
        GList **srv_cfg_list){
    gchar **server_list;

    server_list = g_settings_get_strv(cfg->gs_cfg, "server-list");
    if (server_list){
        for (int i = 0, count = g_strv_length(server_list); i < count; i++){
            gchar *name = NULL;
            GSettings *server;

            gchar *server_path = g_strdup_printf("%s%s/",
                                                 PACKAGE_GSCHEMA_SERVER_PATH,
                                                 server_list[i]);

            server = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_SERVER_ID,
                                                          cfg->gs_backend,
                                                          server_path);
            if (!server) break;

            g_settings_get(server, "name", "ms", &name);
            if (name == NULL) {
                return RET_ERR(_("server-list[%1$s] doesn't have a name"), server_list[i]);
            }
            if (name) {
                GList *lst;

                lst = *srv_cfg_list;
                while (lst) {
                    if (g_ascii_strcasecmp(name, lst->data) == 0){
                        break;
                    }
                    lst = g_list_next(lst);
                }
                if (!lst){
                    *srv_cfg_list = g_list_append(*srv_cfg_list, name);
                }
            }
        }
    }

    return SRN_OK;
}

static SrnRet read_server_config_from_server(GSettings *server,
        SrnServerConfig *cfg){
    /* Read server meta info,
     * NOTE: Server password is not stored in configuration file */
    g_settings_get(server, "name", "ms", &cfg->name);
    settings_get_boolean_ex(server, "tls", &cfg->irc->tls);
    settings_get_boolean_ex(server, "tls-noverify", &cfg->irc->tls_noverify);
    settings_get_string_ex(server, "encoding", &cfg->irc->encoding);
    if (cfg->irc->tls_noverify) {
        cfg->irc->tls = TRUE;
    }

    /* Read server.addrs */
    gchar **addrs = NULL;
    settings_get_strv_ex(server, "addresses", &addrs);
    if (addrs){
        for (int i = 0, count = g_strv_length(addrs); i < count; i++){
            char *addr;

            addr = addrs[i];
            if (addr){
                SrnRet ret;
                SrnServerAddr *srv_addr;

                srv_addr = srn_server_addr_new_from_string(addr);
                ret = srn_server_config_add_addr(cfg, srv_addr);

                if (!RET_IS_OK(ret)){
                    srn_server_addr_free(srv_addr);
                    g_strfreev(addrs);
                    return ret;
                }
            }
        }
        g_strfreev(addrs);
    }

    /* Read server.user */
    {
        SrnRet ret;
        ret = read_user_config_from_server(server, cfg->user);
        if (!RET_IS_OK(ret)){
            return ret;
        }
    }

    /* Read auto join chat list */
    gchar **auto_join = NULL;
    settings_get_strv_ex(server, "auto-join", &auto_join);
    if (auto_join){
        for (int i = 0; i < g_strv_length(auto_join); i++){
            const char *val;

            val = auto_join[i];
            if (!val) continue;

            cfg->auto_join_chat_list = g_list_append(
                    cfg->auto_join_chat_list, g_strdup(val));
        }
        g_strfreev(auto_join);
    }

    /* Read autorun command list */
    gchar **cmds = NULL;
    settings_get_strv_ex(server, "auto-run", &cmds);
    if (cmds){
        for (int i = 0; i < g_strv_length(cmds); i++){
            const char *val;

            val = cmds[i];
            if (!val) continue;

            cfg->auto_run_cmd_list = g_list_append(cfg->auto_run_cmd_list,
                    g_strdup(val));
        }
        g_strfreev(cmds);
    }

    return SRN_OK;
}

static SrnRet read_server_config_from_server_list(SrnSettingsStruct *gs,
        SrnServerConfig *cfg, const char *srv_name){
    int count;
    SrnRet ret;

    gchar **server_list = g_settings_get_strv(gs->gs_cfg, "server-list");
    count = g_strv_length(server_list);
    for (int i = 0; i < count; i++){
        char *name = NULL;
        GSettings *server;

        gchar *server_path = g_strdup_printf("%s%s/",
                                             PACKAGE_GSCHEMA_SERVER_PATH,
                                             server_list[i]);
        server = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_SERVER_ID,
                                                      gs->gs_backend,
                                                      server_path);
        if (!server){
            g_free(server_path);
            break;
        };

        g_settings_get(server, "name", "ms", &name);
        if (g_strcmp0(srv_name, name) != 0){
            g_free(name);
            g_free(server_path);
            g_object_unref(server);
            continue;
        };

        DBG_FR("Read: server-list.[name = %s], srv_name: %s", name, srv_name);
        ret = read_server_config_from_server(server, cfg);
        g_free(name);
        g_free(server_path);
        g_object_unref(server);
        if (!RET_IS_OK(ret)) return ret;
    }
    g_strfreev(server_list);

    return SRN_OK;
}

static SrnRet read_server_config_from_cfg(SrnSettingsStruct *cfg, SrnServerConfig *srv_cfg,
        const char *srv_name){
    SrnRet ret;
    GSettings *server;

    /* Read server */
    server = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_SERVER_ID,
                                                  cfg->gs_backend,
                                                  PACKAGE_GSCHEMA_SERVER_PATH);
    if (server){
        DBG_FR("Read: server, srv_name: %s", srv_name);
        ret = read_server_config_from_server(server, srv_cfg);
        g_object_unref(server);
        if (!RET_IS_OK(ret)) return ret;
    }

    /* Read server_list[name = srv_name] */
    ret = read_server_config_from_server_list(cfg, srv_cfg, srv_name);
    if (!RET_IS_OK(ret)) return ret;

    return SRN_OK;
}

static SrnRet read_chat_config_from_chat(GSettings *chat, SrnChatConfig *cfg){
     settings_get_boolean_ex(chat, "notify", &cfg->ui->notify);
     settings_get_boolean_ex(chat, "show-topic", &cfg->ui->show_topic);
     settings_get_boolean_ex(chat, "show-avatar", &cfg->ui->show_avatar);
     settings_get_boolean_ex(chat, "show-user-list", &cfg->ui->show_user_list);
     settings_get_boolean_ex(chat, "render-mirc-color", &cfg->render_mirc_color);
     settings_get_boolean_ex(chat, "preview-url", &cfg->ui->preview_url);
     settings_get_boolean_ex(chat, "auto-preview-url", &cfg->ui->auto_preview_url);
     settings_get_string_ex(chat, "nick-completion-suffix", &cfg->ui->nick_completion_suffix);

    /* Read autorun command list */
    gchar **cmds = NULL;
    settings_get_strv_ex(chat, "auto-run", &cmds);
    if (cmds){
        for (int i = 0; i < g_strv_length(cmds); i++){
            const char *val;

            val = cmds[i];
            if (!val) continue;

            cfg->auto_run_cmd_list = g_list_append(cfg->auto_run_cmd_list,
                    g_strdup(val));
        }
    }

    g_strfreev(cmds);
    return SRN_OK;
}

static SrnRet read_chat_config_from_chat_list(SrnSettingsStruct *gs,
        gchar **chat_list, SrnChatConfig *cfg, const char *chat_name){
    SrnRet ret;

    for (int i = 0, count = g_strv_length(chat_list); i < count; i++){
        const char *name = NULL;
        GSettings *chat;

        gchar *chat_path = g_strdup_printf("%s%s/",
                                           PACKAGE_GSCHEMA_CHAT_PATH,
                                           chat_list[i]);
        chat = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_CHAT_ID,
                                                    gs->gs_backend,
                                                    chat_path);

        if (!chat){
            g_free(chat_path);
            break;
        };
        name = chat_list[i];

        DBG_FR("Read: chat-list.%s, chat_name: %s", name, chat_name);
        ret = read_chat_config_from_chat(chat, cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_chat_config_from_server_chat_list(SrnSettingsStruct *gs,
        const char *srv_name, gchar **chat_list, SrnChatConfig *cfg, const char *chat_name){
    SrnRet ret;

    for (int i = 0, count = g_strv_length(chat_list); i < count; i++){
        const char *name = NULL;
        GSettings *chat;

        gchar *chat_path = g_strdup_printf("%s%s/Chat/%s/",
                                           PACKAGE_GSCHEMA_SERVER_PATH,
                                           srv_name,
                                           chat_list[i]);
        chat = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_CHAT_ID,
                                                    gs->gs_backend,
                                                    chat_path);

        if (!chat){
            g_free(chat_path);
            break;
        };
        name = chat_list[i];

        DBG_FR("Read: server.%s.chat-list.%s, chat_name: %s", srv_name, name, chat_name);
        ret = read_chat_config_from_chat(chat, cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_chat_config_from_cfg(SrnSettingsStruct *cfg, SrnChatConfig *chat_cfg,
        const char *srv_name, const char *chat_name){
    SrnRet ret;

    /* Read server.chat */
    GSettings *chat;

    chat = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_CHAT_ID,
                                                cfg->gs_backend,
                                                PACKAGE_GSCHEMA_CHAT_PATH);
    if (chat){
        DBG_FR("Read: server.chat, srv_name: %s, chat_name: %s", srv_name, chat_name);
        ret = read_chat_config_from_chat(chat, chat_cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    /* Read server.chat_list[name = chat_name] */
    if (chat_name){
        gchar **chat_list = NULL;

        settings_get_strv_ex(chat, "chat-list", &chat_list);
        if (chat_list){
            ret = read_chat_config_from_chat_list(cfg, chat_list, chat_cfg, chat_name);
            if (!RET_IS_OK(ret)) return ret;
        }
    }

    if (srv_name){
        gchar **server_list;

        server_list = g_settings_get_strv(cfg->gs_cfg, "server-list");
        if (server_list){
            for (int i = 0, count = g_strv_length(server_list); i < count; i++){
                const char *name = NULL;
                GSettings *chat;

                name = server_list[i];
                if (g_strcmp0(srv_name, name) != 0) continue;

                /* Read server_list.[name = srv_name].chat */

                gchar *server_global_chat_path = g_strdup_printf("%s%s/Chat/",
                                                                 PACKAGE_GSCHEMA_SERVER_PATH,
                                                                 server_list[i]);
                chat = g_settings_new_with_backend_and_path(PACKAGE_GSCHEMA_CHAT_ID,
                                                            cfg->gs_backend,
                                                            server_global_chat_path);
                if (chat){
                    DBG_FR("Read server-list.[name = %s].chat, srv_name: %s, chat_name: %s",
                            name, srv_name, chat_name);
                    ret = read_chat_config_from_chat(chat, chat_cfg);
                    if (!RET_IS_OK(ret)) return ret;
                }
                g_free(server_global_chat_path);

                /* Read server_list.[name = srv_name].chat_list[name = chat_name] */
                if (chat_name){
                    gchar **chat_list = NULL;
                    settings_get_strv_ex(chat, "chat-list", &chat_list);
                    if (chat_list){
                        ret = read_chat_config_from_server_chat_list(cfg, name, chat_list, chat_cfg, chat_name);
                        g_strfreev(chat_list);
                        if (!RET_IS_OK(ret)) return ret;
                    }
                }
                g_object_unref(chat);
            }

            g_strfreev(server_list);
        }
    }

    g_object_unref(chat);
    return SRN_OK;
}

static SrnRet read_user_config_from_server(GSettings *server, SrnUserConfig *cfg){

    // NOTE: Login password is not stored in configuration file
    gint method = g_settings_get_enum(server, "login-method");
    // NOTE: 5 == Unknown method
    if (method != 5) cfg->login->method = method;
    settings_get_string_ex(server, "login-certificate", &cfg->login->cert_file);

    settings_get_string_ex(server, "user-nickname", &cfg->nick);
    settings_get_string_ex(server, "user-username", &cfg->username);
    settings_get_string_ex(server, "user-realname", &cfg->realname);

    settings_get_string_ex(server, "part-message", &cfg->part_message);
    settings_get_string_ex(server, "kick-message", &cfg->kick_message);
    settings_get_string_ex(server, "away-message", &cfg->away_message);
    settings_get_string_ex(server, "quit-message", &cfg->quit_message);

    return SRN_OK;
}

static void settings_get_string_ex(GSettings *settings, const char* key, char **value){
    char *constval = NULL;

    g_settings_get(settings, key, "ms", &constval);
    if (constval != NULL) *value = constval;
}

static void settings_get_boolean_ex(GSettings *settings, const char* key, gboolean *value){
    GVariant *maybe = g_settings_get_value(settings, key);
    GVariant *bool_v = g_variant_get_maybe(maybe);
    if (bool_v != NULL) *value = g_variant_get_boolean(bool_v);
}

static void settings_get_strv_ex(GSettings *settings, const char* key, gchar ***value){
    GVariant *maybe = g_settings_get_value(settings, key);
    GVariant *strv_v = g_variant_get_maybe(maybe);
    if (strv_v != NULL) *value = g_variant_dup_strv(strv_v, NULL);
}
