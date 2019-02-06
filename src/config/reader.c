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
#include <libconfig.h>

#include "core/core.h"
#include "config/config.h"
#include "i18n.h"
#include "utils.h"

/* Libconfig helpers */
static int config_lookup_string_ex(const config_t *config, const char *path, char **value);
static int config_setting_lookup_string_ex(const config_setting_t *config, const char *name, char **value);
static char* config_setting_get_string_elem_ex(const config_setting_t *setting, int index);
static int config_lookup_bool_ex(const config_t *config, const char *name, bool *value);
static int config_setting_lookup_bool_ex(const config_setting_t *config, const char *name, bool *value);

/* Configuration readers for various config structures */
static SrnRet read_log_config_from_cfg(config_t *cfg, SrnLoggerConfig *log_cfg);
static SrnRet read_log_targets_from_log(config_setting_t *log, const char *name, GList **lst);

static SrnRet read_application_config_from_cfg(config_t *cfg, SrnApplicationConfig *app_cfg);

static SrnRet read_server_config_list_from_cfg(config_t *cfg, GList **srv_cfg_list);
static SrnRet read_server_config_from_server(config_setting_t *server, SrnServerConfig *cfg);
static SrnRet read_server_config_from_server_list(config_setting_t *server_list, SrnServerConfig *cfg, const char *srv_name);
static SrnRet read_server_config_from_cfg(config_t *cfg, SrnServerConfig *srv_cfg, const char *srv_name);

static SrnRet read_chat_config_from_chat(config_setting_t *chat, SrnChatConfig *cfg);
static SrnRet read_chat_config_from_chat_list(config_setting_t *chat_list, SrnChatConfig *cfg, const char *chat_name);
static SrnRet read_chat_config_from_cfg(config_t *cfg, SrnChatConfig *chat_cfg, const char *srv_name, const char *chat_name);

static SrnRet read_user_config_from_user(config_setting_t *user, SrnUserConfig *cfg);

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SrnRet srn_config_manager_read_log_config(SrnConfigManager *mgr,
        SrnLoggerConfig *cfg){
    SrnRet ret;

    ret = read_log_config_from_cfg(&mgr->system_cfg, cfg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read log config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }
    ret = read_log_config_from_cfg(&mgr->user_cfg, cfg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read log config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet srn_config_manager_read_application_config(SrnConfigManager *mgr,
        SrnApplicationConfig *cfg){
    SrnRet ret;

    ret = read_application_config_from_cfg(&mgr->system_cfg, cfg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read application config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }
    ret = read_application_config_from_cfg(&mgr->user_cfg, cfg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read application config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet srn_config_manager_read_server_config_list(SrnConfigManager *mgr,
        GList **srv_cfg_list){
    SrnRet ret;

    ret = read_server_config_list_from_cfg(&mgr->system_cfg, srv_cfg_list);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server list config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }
    ret = read_server_config_list_from_cfg(&mgr->user_cfg, srv_cfg_list);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server list config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet srn_config_manager_read_server_config(SrnConfigManager *mgr,
        SrnServerConfig *cfg, const char *srv_name){
    SrnRet ret;

    ret = read_server_config_from_cfg(&mgr->system_cfg, cfg, srv_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }
    ret = read_server_config_from_cfg(&mgr->user_cfg, cfg, srv_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
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

    ret = read_chat_config_from_cfg(&mgr->system_cfg, cfg, srv_name, chat_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read chat config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->system_cfg)),
                RET_MSG(ret));
    }

    ret = read_chat_config_from_cfg(&mgr->user_cfg, cfg, srv_name, chat_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read chat config in %1$s: %2$s"),
                config_setting_source_file(config_root_setting(&mgr->user_cfg)),
                RET_MSG(ret));
    }

    return SRN_OK;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static SrnRet read_log_config_from_cfg(config_t *cfg, SrnLoggerConfig *log_cfg){
    SrnRet ret;
    config_setting_t *log;

    log = config_lookup(cfg, "log");
    if (log){
        config_setting_lookup_bool_ex(log, "prompt-color", &log_cfg->prompt_color);
        config_setting_lookup_bool_ex(log, "prompt-file", &log_cfg->prompt_file);
        config_setting_lookup_bool_ex(log, "prompt-function", &log_cfg->prompt_function);
        config_setting_lookup_bool_ex(log, "prompt-line", &log_cfg->prompt_line);

        ret = read_log_targets_from_log(log, "debug-targets", &log_cfg->debug_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "info-targets", &log_cfg->info_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "warn-targets", &log_cfg->warn_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "error-targets", &log_cfg->error_targets);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_log_targets_from_log(config_setting_t *log, const char *name,
        GList **lst){
    config_setting_t *targets;

    targets = config_setting_lookup(log, name);
    if (!targets) return SRN_OK;

    int count = config_setting_length(targets);
    for (int i = 0; i < count; i++){
        const char *val;
        config_setting_t *target;

        target = config_setting_get_elem(targets, i);
        if (!target) break;

        val = config_setting_get_string(target);
        if (!val) continue;

        *lst = g_list_append(*lst, g_strdup(val));
    }

    return SRN_OK;
}
static SrnRet read_application_config_from_cfg(config_t *cfg,
        SrnApplicationConfig *app_cfg){
    config_lookup_string_ex(cfg, "id", &app_cfg->id);
    config_lookup_string_ex(cfg, "theme", &app_cfg->ui->theme);
    config_lookup_bool_ex(cfg, "csd", &app_cfg->ui->window.csd);
    config_lookup_bool_ex(cfg, "send-on-ctrl-enter",
            &app_cfg->ui->window.send_on_ctrl_enter);
    config_lookup_bool_ex(cfg, "exit-on-close",
            &app_cfg->ui->window.exit_on_close);

    /* Read auto connect server list */
    config_setting_t *auto_connect;
    auto_connect = config_lookup(cfg, "auto-connect");
    if (auto_connect){
        for (int i = 0; i < config_setting_length(auto_connect); i++){
            const char *val;
            config_setting_t *server;

            server = config_setting_get_elem(auto_connect, i);
            if (!server) continue;
            val = config_setting_get_string(server);
            if (!val) continue;

            app_cfg->auto_connect_srv_list = g_list_append(
                    app_cfg->auto_connect_srv_list, g_strdup(val));
        }
    }

    return SRN_OK;
}

static SrnRet read_server_config_list_from_cfg(config_t *cfg,
        GList **srv_cfg_list){
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server-list");
    if (server_list){
        for (int i = 0, count = config_setting_length(server_list); i < count; i++){
            char *name = NULL;
            config_setting_t *server;

            server = config_setting_get_elem(server_list, i);
            if (!server) break;

            if (config_setting_lookup_string_ex(server, "name", &name) != CONFIG_TRUE) {
                return RET_ERR(_("server-list[%1$d] doesn't have a name"), i);
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

static SrnRet read_server_config_from_server(config_setting_t *server,
        SrnServerConfig *cfg){
    /* Read server meta info */
    config_setting_lookup_string_ex(server, "name", &cfg->name);
    config_setting_lookup_string_ex(server, "password", &cfg->passwd);
    config_setting_lookup_bool_ex(server, "tls", &cfg->irc->tls);
    config_setting_lookup_bool_ex(server, "tls-noverify", &cfg->irc->tls_noverify);
    config_setting_lookup_string_ex(server, "encoding", &cfg->irc->encoding);
    if (cfg->irc->tls_noverify) {
        cfg->irc->tls = TRUE;
    }

    /* Read server.addrs */
    config_setting_t *addrs;
    addrs = config_setting_lookup(server, "addresses");
    if (addrs){
        for (int i = 0, count = config_setting_length(addrs); i < count; i++){
            char *addr;

            addr = config_setting_get_string_elem_ex(addrs, i);
            if (addr){
                SrnRet ret;
                SrnServerAddr *srv_addr;

                srv_addr = srn_server_addr_new_from_string(addr);
                ret = srn_server_config_add_addr(cfg, srv_addr);
                g_free(addr);

                if (!RET_IS_OK(ret)){
                    srn_server_addr_free(srv_addr);
                    return ret;
                }
            }
        }
    }

    /* Read server.user */
    config_setting_t *user;
    user = config_setting_lookup(server, "user");
    if (user){
        SrnRet ret;
        ret = read_user_config_from_user(user, cfg->user);
        if (!RET_IS_OK(ret)){
            return ret;
        }
    }

    /* Read auto join chat list */
    config_setting_t *auto_join;
    auto_join = config_setting_lookup(server, "auto-join");
    if (auto_join){
        for (int i = 0; i < config_setting_length(auto_join); i++){
            const char *val;
            config_setting_t *chat;

            chat = config_setting_get_elem(auto_join, i);
            if (!chat) continue;
            val = config_setting_get_string(chat);
            if (!val) continue;

            cfg->auto_join_chat_list = g_list_append(
                    cfg->auto_join_chat_list, g_strdup(val));
        }
    }

    /* Read autorun command list */
    config_setting_t *cmds;
    cmds = config_setting_lookup(server, "auto-run");
    if (cmds){
        for (int i = 0; i < config_setting_length(cmds); i++){
            const char *val;
            config_setting_t *cmd;

            cmd = config_setting_get_elem(cmds, i);
            if (!cmd) continue;
            val = config_setting_get_string(cmd);
            if (!val) continue;

            cfg->auto_run_cmd_list = g_list_append(cfg->auto_run_cmd_list,
                    g_strdup(val));
        }
    }

    return SRN_OK;
}

static SrnRet read_server_config_from_server_list(config_setting_t *server_list,
        SrnServerConfig *cfg, const char *srv_name){
    int count;
    SrnRet ret;

    count = config_setting_length(server_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *server;

        server = config_setting_get_elem(server_list, i);
        if (!server) break;

        config_setting_lookup_string(server, "name", &name);
        if (g_strcmp0(srv_name, name) != 0) continue;

        DBG_FR("Read: server-list.[name = %s], srv_name: %s", name, srv_name);
        ret = read_server_config_from_server(server, cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_server_config_from_cfg(config_t *cfg, SrnServerConfig *srv_cfg,
        const char *srv_name){
    SrnRet ret;
    config_setting_t *server;

    /* Read server */
    server = config_lookup(cfg, "server");
    if (server){
        DBG_FR("Read: server, srv_name: %s", srv_name);
        ret = read_server_config_from_server(server, srv_cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    /* Read server_list[name = srv_name] */
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server-list");
    if (server_list){
        ret = read_server_config_from_server_list(server_list, srv_cfg, srv_name);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_chat_config_from_chat(config_setting_t *chat, SrnChatConfig *cfg){
    config_setting_lookup_bool_ex(chat, "notify", &cfg->ui->notify);
    config_setting_lookup_bool_ex(chat, "show-topic", &cfg->ui->show_topic);
    config_setting_lookup_bool_ex(chat, "show-avatar", &cfg->ui->show_avatar);
    config_setting_lookup_bool_ex(chat, "show-user-list", &cfg->ui->show_user_list);
    config_setting_lookup_bool_ex(chat, "render-mirc-color", &cfg->render_mirc_color);
    config_setting_lookup_bool_ex(chat, "preview-url", &cfg->ui->preview_url);
    config_setting_lookup_bool_ex(chat, "auto-preview-url", &cfg->ui->auto_preview_url);
    config_setting_lookup_string_ex(chat, "nick-completion-suffix", &cfg->ui->nick_completion_suffix);

    /* Read autorun command list */
    config_setting_t *cmds;
    cmds = config_setting_lookup(chat, "auto-run");
    if (cmds){
        for (int i = 0; i < config_setting_length(cmds); i++){
            const char *val;
            config_setting_t *cmd;

            cmd = config_setting_get_elem(cmds, i);
            if (!cmd) continue;
            val = config_setting_get_string(cmd);
            if (!val) continue;

            cfg->auto_run_cmd_list = g_list_append(cfg->auto_run_cmd_list,
                    g_strdup(val));
        }
    }

    return SRN_OK;
}

static SrnRet read_chat_config_from_chat_list(config_setting_t *chat_list,
        SrnChatConfig *cfg, const char *chat_name){
    SrnRet ret;

    for (int i = 0, count = config_setting_length(chat_list); i < count; i++){
        const char *name = NULL;
        config_setting_t *chat;

        chat = config_setting_get_elem(chat_list, i);
        if (!chat) break;
        if (config_setting_lookup_string(chat, "name", &name) != CONFIG_TRUE){
            continue;
        }
        if (g_strcmp0(chat_name, name) != 0) continue;

        DBG_FR("Read: chat-list.%s, chat_name: %s", name, chat_name);
        ret = read_chat_config_from_chat(chat, cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_chat_config_from_cfg(config_t *cfg, SrnChatConfig *chat_cfg,
        const char *srv_name, const char *chat_name){
    SrnRet ret;

    /* Read server.chat */
    config_setting_t *chat;

    chat = config_lookup(cfg, "server.chat");
    if (chat){
        DBG_FR("Read: server.chat, srv_name: %s, chat_name: %s", srv_name, chat_name);
        ret = read_chat_config_from_chat(chat, chat_cfg);
        if (!RET_IS_OK(ret)) return ret;
    }

    /* Read server.chat_list[name = chat_name] */
    if (chat_name){
        config_setting_t *chat_list;

        chat_list = config_lookup(cfg, "server.chat-list");
        if (chat_list){
            ret = read_chat_config_from_chat_list(chat_list, chat_cfg, chat_name);
            if (!RET_IS_OK(ret)) return ret;
        }
    }

    if (srv_name){
        config_setting_t *server_list;

        server_list = config_lookup(cfg, "server-list");
        if (server_list){
            for (int i = 0, count = config_setting_length(server_list); i < count; i++){
                const char *name = NULL;
                config_setting_t *server;
                config_setting_t *chat;

                server = config_setting_get_elem(server_list, i);
                if (!server) break;

                config_setting_lookup_string(server, "name", &name);
                if (g_strcmp0(srv_name, name) != 0) continue;

                /* Read server_list.[name = srv_name].chat */
                chat = config_setting_lookup(server, "chat");
                if (chat){
                    DBG_FR("Read server-list.[name = %s].chat, srv_name: %s, chat_name: %s",
                            name, srv_name, chat_name);
                    ret = read_chat_config_from_chat(chat, chat_cfg);
                    if (!RET_IS_OK(ret)) return ret;
                }

                /* Read server_list.[name = srv_name].chat_list[name = chat_name] */
                if (chat_name){
                    config_setting_t *chat_list;

                    chat_list = config_setting_lookup(server, "server.chat-list");
                    if (chat_list){
                        ret = read_chat_config_from_chat_list(chat_list, chat_cfg, chat_name);
                        if (!RET_IS_OK(ret)) return ret;
                    }
                }
            }
        }
    }

    return SRN_OK;
}

static SrnRet read_user_config_from_user(config_setting_t *user, SrnUserConfig *cfg){
    config_setting_t *login;

    // Read user.login
    login = config_setting_lookup(user, "login");
    if (login) {
        const char *method = NULL;

        config_setting_lookup_string(login, "method", &method);
        cfg->login->method = srn_login_method_from_string(method);

        config_setting_lookup_string_ex(login, "nickserv-password", &cfg->login->nickserv_password);
        config_setting_lookup_string_ex(login, "msg-nickserv-password", &cfg->login->msg_nickserv_password);
        config_setting_lookup_string_ex(login, "sasl-plain-identify", &cfg->login->sasl_plain_identify);
        config_setting_lookup_string_ex(login, "sasl-plain-password", &cfg->login->sasl_plain_password);
        config_setting_lookup_string_ex(login, "sasl-certificate", &cfg->login->sasl_certificate_file);
    }

    config_setting_lookup_string_ex(user, "nickname", &cfg->nick);
    config_setting_lookup_string_ex(user, "username", &cfg->username);
    config_setting_lookup_string_ex(user, "realname", &cfg->realname);

    config_setting_lookup_string_ex(user, "part-message", &cfg->part_message);
    config_setting_lookup_string_ex(user, "kick-message", &cfg->kick_message);
    config_setting_lookup_string_ex(user, "away-message", &cfg->away_message);
    config_setting_lookup_string_ex(user, "quit-message", &cfg->quit_message);

    return SRN_OK;
}

/**
 * @brief Wrapper of config_lookup_string(), This function will allocate a new
 * string, you should free it by yourself
 *
 * @param config
 * @param path
 * @param value
 *
 * @return
 */
static int config_lookup_string_ex(const config_t *config, const char *path, char **value){
    int ret;
    const char *constval = NULL;

    ret = config_lookup_string(config, path, &constval);
    if (ret == CONFIG_TRUE){
        str_assign(value, constval);
    }

    return ret;
}

/**
 * @brief Wrapper of config_setting_lookup_string(), This function will
 *      allocate a new string, you should free it by yourself
 *
 * @param config
 * @param path
 * @param value
 *
 * @return
 */
static int config_setting_lookup_string_ex(const config_setting_t *config,
        const char *name, char **value){
    int ret;
    const char *constval = NULL;

    ret = config_setting_lookup_string(config, name, &constval);
    if (ret == CONFIG_TRUE){
        str_assign(value, constval);
    }

    return ret;
}

static char* config_setting_get_string_elem_ex(const config_setting_t *setting,
        int index){
    return g_strdup(config_setting_get_string_elem(setting, index));
}

static int config_lookup_bool_ex(const config_t *config, const char *name,
        bool *value){
    int intval;
    int ret;

    ret = config_lookup_bool(config, name, &intval);
    if (ret == CONFIG_TRUE){
        *value = (bool)intval;
    }

    return ret;
}

/**
 * @brief Wrapper of config_setting_lookup_bool(), The "bool" in libconfig is
 * actually an integer, This function transform it to fit bool (alias of
 * gboolean) defined in "srain.h"
 *
 * @param config
 * @param name
 * @param value
 *
 * @return
 */
static int config_setting_lookup_bool_ex(const config_setting_t *config,
        const char *name, bool *value){
    int intval;
    int ret;

    ret = config_setting_lookup_bool(config, name, &intval);
    if (ret == CONFIG_TRUE){
        *value = (bool)intval;
    }

    return ret;
}
