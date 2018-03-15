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

// FIXME: config

/* Libconfig helpers */
static int config_lookup_string_ex(const config_t *config, const char *path, char **value);
static int config_setting_lookup_string_ex(const config_setting_t *config, const char *name, char **value);
static char* config_setting_get_string_elem_ex(const config_setting_t *setting, int index);
static int config_lookup_bool_ex(const config_t *config, const char *name, bool *value);
static int config_setting_lookup_bool_ex(const config_setting_t *config, const char *name, bool *value);

/* Configuration readers for various config structures */
static SrnRet read_log_config_from_cfg(config_t *cfg, SrnLoggerConfig *log_cfg);
static SrnRet read_log_targets_from_log(config_setting_t *log, const char *name, GSList **lst);

static SrnRet read_application_config_from_cfg(config_t *cfg, SrnApplicationConfig *app_cfg);
static SrnRet read_server_config_list_from_cfg(config_t *cfg, GSList **srv_cfg_list);

static SrnRet read_server_config_from_server(config_setting_t *server, SrnServerConfig *cfg);
static SrnRet read_server_config_from_server_list(config_setting_t *server_list, SrnServerConfig *cfg, const char *srv_name);
static SrnRet read_server_config_from_cfg(config_t *cfg, SrnServerConfig *srv_cfg, const char *srv_name);

static SrnRet read_chat_config_from_chat(config_setting_t *chat, SrnChatConfig *cfg);
static SrnRet read_chat_config_from_chat_list(config_setting_t *chat_list, SrnChatConfig *cfg, const char *chat_name);
static SrnRet read_chat_config_from_cfg(config_t *cfg, SrnChatConfig *chat_cfg, const char *srv_name, const char *chat_name);

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
        GSList **srv_cfg_list){
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
        config_setting_lookup_bool_ex(log, "prompt_color", &log_cfg->prompt_color);
        config_setting_lookup_bool_ex(log, "prompt_file", &log_cfg->prompt_file);
        config_setting_lookup_bool_ex(log, "prompt_function", &log_cfg->prompt_function);
        config_setting_lookup_bool_ex(log, "prompt_line", &log_cfg->prompt_line);

        ret = read_log_targets_from_log(log, "debug_targets", &log_cfg->debug_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "info_targets", &log_cfg->info_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "warn_targets", &log_cfg->warn_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "error_targets", &log_cfg->error_targets);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_log_targets_from_log(config_setting_t *log, const char *name,
        GSList **lst){
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

        *lst = g_slist_append(*lst, g_strdup(val));
    }

    return SRN_OK;
}
static SrnRet read_application_config_from_cfg(config_t *cfg,
        SrnApplicationConfig *app_cfg){
    config_lookup_string_ex(cfg, "id", &app_cfg->id);
    config_lookup_string_ex(cfg, "theme", &app_cfg->ui->theme);

    return SRN_OK;
}

static SrnRet read_server_config_list_from_cfg(config_t *cfg,
        GSList **srv_cfg_list){
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server_list");
    if (server_list){
        for (int i = 0, count = config_setting_length(server_list); i < count; i++){
            const char *name = NULL;
            GSList *lst;
            config_setting_t *server;
            SrnRet ret;
            SrnServerConfig *srv_cfg;

            server = config_setting_get_elem(server_list, i);
            if (!server) break;

            if (config_setting_lookup_string(server, "name", &name) != CONFIG_TRUE) {
                return RET_ERR(_("server_list[%1$d] doesn't have a name"), i);
            }

            srv_cfg = NULL;
            lst = *srv_cfg_list;
            while (lst){
                SrnServerConfig *tmp;

                tmp = lst->data;
                if (g_ascii_strcasecmp(tmp->name, name) == 0){
                    srv_cfg = tmp;
                    break;
                }
                lst = g_slist_next(lst);
            }
            if (!srv_cfg) {
                srv_cfg = srn_server_config_new(name);
                srv_cfg->predefined = TRUE;
            }

            ret = read_server_config_from_cfg(cfg, srv_cfg, name);
            if (!RET_IS_OK(ret)){
                srn_server_config_free(srv_cfg);
                return ret;
            }

            *srv_cfg_list = g_slist_append(*srv_cfg_list, srv_cfg);
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
    config_setting_lookup_bool_ex(server, "tls_noverify", &cfg->irc->tls_noverify);
    config_setting_lookup_string_ex(server, "encoding", &cfg->irc->encoding);
    if (cfg->irc->tls_noverify) {
        cfg->irc->tls = TRUE;
    }

    /* Read server.addrs */
    config_setting_t *addrs;
    addrs = config_setting_lookup(server, "addrs");
    if (addrs){
        for (int i = 0, count = config_setting_length(addrs); i < count; i++){
            char *addr;

            addr = config_setting_get_string_elem_ex(addrs, i);
            if (addr){
                int port;
                char *tmp;
                const char *host;

                host = addr;
                port = 0;
                tmp = strchr(addr, ':');
                if (tmp) {
                    *tmp++ = '\0';
                    port = g_ascii_strtoull(tmp, NULL, 10);
                }
                srn_server_config_add_addr(cfg, host, port);
                g_free(addr);
            }
        }
    }

    /* Read server.user */
    config_setting_t *user;
    user = config_setting_lookup(server, "user");
    if (user){
        char *login_method = NULL;

        config_setting_lookup_string_ex(user, "login_method", &login_method);
        if (login_method) {
            cfg->login_method = login_method_from_string(login_method);
            g_free(login_method);
        }

        config_setting_lookup_string_ex(user, "nickname", &cfg->nickname);
        config_setting_lookup_string_ex(user, "username", &cfg->username);
        config_setting_lookup_string_ex(user, "realname", &cfg->realname);
        config_setting_lookup_string_ex(user, "password", &cfg->user_passwd);
    }

    /* Read server.default_messages */
    config_setting_t *default_messages;
    default_messages = config_setting_lookup(server, "default_messages");
    if (default_messages){
        config_setting_lookup_string_ex(default_messages, "part", &cfg->part_message);
        config_setting_lookup_string_ex(default_messages, "kick", &cfg->kick_message);
        config_setting_lookup_string_ex(default_messages, "away", &cfg->away_message);
        config_setting_lookup_string_ex(default_messages, "quit", &cfg->quit_message);
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

        DBG_FR("Read: server_list.[name = %s], srv_name: %s", name, srv_name);
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

    server_list = config_lookup(cfg, "server_list");
    if (server_list){
        ret = read_server_config_from_server_list(server_list, srv_cfg, srv_name);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_chat_config_from_chat(config_setting_t *chat, SrnChatConfig *cfg){
    config_setting_lookup_bool_ex(chat, "notify", &cfg->ui->notify);
    config_setting_lookup_bool_ex(chat, "show_topic", &cfg->ui->show_topic);
    config_setting_lookup_bool_ex(chat, "show_avatar", &cfg->ui->show_avatar);
    config_setting_lookup_bool_ex(chat, "show_user_list", &cfg->ui->show_user_list);
    config_setting_lookup_bool_ex(chat, "render_mirc_color", &cfg->render_mirc_color);

    /* Read url handlers */
    config_setting_lookup_bool_ex(chat, "url_handlers.http.fetch_image", &cfg->ui->preview_image);
    // TODO

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

        DBG_FR("Read: chat_list.%s, chat_name: %s", name, chat_name);
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

        chat_list = config_lookup(cfg, "server.chat_list");
        if (chat_list){
            ret = read_chat_config_from_chat_list(chat_list, chat_cfg, chat_name);
            if (!RET_IS_OK(ret)) return ret;
        }
    }

    if (srv_name){
        config_setting_t *server_list;

        server_list = config_lookup(cfg, "server_list");
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
                    DBG_FR("Read server_list.[name = %s].chat, srv_name: %s, chat_name: %s",
                            name, srv_name, chat_name);
                    ret = read_chat_config_from_chat(chat, chat_cfg);
                    if (!RET_IS_OK(ret)) return ret;
                }

                /* Read server_list.[name = srv_name].chat_list[name = chat_name] */
                if (chat_name){
                    config_setting_t *chat_list;

                    chat_list = config_setting_lookup(server, "server.chat_list");
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
    if (constval){
        *value = g_strdup(constval);
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
    str_assign(value, constval);

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
