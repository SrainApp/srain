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
 * @file prefs.c
 * @brief Libconfig based configure file reader
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-05-14
 */


#include <glib.h>
#include <libconfig.h>
#include <string.h>

#include "prefs.h"
#include "srain.h"
#include "log.h"
#include "file_helper.h"
#include "i18n.h"

config_t user_cfg;
config_t builtin_cfg;

static SrnRet prefs_read_file(config_t *cfg, const char *file);

static int config_lookup_string_ex(const config_t *config, const char *path, char **value);
static int config_setting_lookup_bool_ex(const config_setting_t *config, const char *name, bool *value);
static int config_setting_lookup_string_ex(const config_setting_t *config, const char *name, char **value);

static SrnRet read_log_prefs_from_cfg(config_t *cfg, LogPrefs *prefs);
static SrnRet read_log_targets_from_log(config_setting_t *log, const char *name, GSList **lst);

static SrnRet read_sirc_prefs_from_irc(config_setting_t *irc, SircPrefs *prefs);
static SrnRet read_sirc_prefs_from_server_list(config_setting_t *server_list, SircPrefs *prefs, const char *srv_name);
static SrnRet read_sirc_prefs_from_cfg(config_t *cfg, SircPrefs *prefs, const char *srv_name);

static SrnRet read_server_prefs_from_server(config_setting_t *server, ServerPrefs *prefs);
static SrnRet read_server_prefs_from_server_list(config_setting_t *server_list, ServerPrefs *prefs);
static SrnRet read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs);
static SrnRet read_server_prefs_list_from_cfg(config_t *cfg);

static SrnRet read_sui_prefs_from_chat(config_setting_t *chat, SuiPrefs *prefs);
static SrnRet read_sui_prefs_from_chat_list(config_setting_t *chat_list, SuiPrefs *prefs, const char *chat_name);
static SrnRet read_sui_prefs_from_cfg(config_t *cfg, SuiPrefs *prefs, const char *srv_name, const char *chat_name);

void prefs_init(){
    config_init(&user_cfg);
    config_init(&builtin_cfg);
};

void prefs_finalize(){
    config_destroy(&user_cfg);
    config_destroy(&builtin_cfg);
};

SrnRet prefs_read(){
    char *path;
    SrnRet ret;

    path = get_system_config_file("builtin.cfg");
    if (!path){
        return RET_ERR(_("System config file %s not found"), "builtin.cfg");
    }

    ret = prefs_read_file(&builtin_cfg, path);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading %s: %s"), path, RET_MSG(ret));
    }

    path = get_config_file("srain.cfg");
    if (!path){
        // It is not an error
        return RET_OK(_("User config file %s not found"), "srain.cfg");
    }

    ret = prefs_read_file(&user_cfg, path);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while reading %s: %s"), path, RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet prefs_read_log_prefs(LogPrefs *prefs){
    SrnRet ret;

    ret = read_log_prefs_from_cfg(&builtin_cfg, prefs);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read log prefs in %s: %s"), "builtin.cfg", RET_MSG(ret));
    }
    ret = read_log_prefs_from_cfg(&user_cfg, prefs);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read log prefs in %s: %s"), "srain.cfg", RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet prefs_read_sui_app_prefs(SuiAppPrefs *prefs){
    // TODO: read_sui_app_prefs_from_cfg()
    config_lookup_string_ex(&builtin_cfg, "application.theme", &prefs->theme);
    config_lookup_string_ex(&user_cfg, "application.theme", &prefs->theme);

    return SRN_OK;
}

SrnRet prefs_read_server_prefs_list(){
    SrnRet ret;

    ret = read_server_prefs_list_from_cfg(&builtin_cfg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server prefs list in %s: %s"), "builtin.cfg", RET_MSG(ret));
    }
    ret = read_server_prefs_list_from_cfg(&user_cfg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server prefs list in %s: %s"), "srain.cfg", RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet prefs_read_server_prefs(ServerPrefs *prefs){
    SrnRet ret;

    ret = read_server_prefs_from_cfg(&builtin_cfg, prefs);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server prefs in %s: %s"), "builtin.cfg", RET_MSG(ret));
    }
    read_server_prefs_from_cfg(&user_cfg, prefs);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read server prefs in %s: %s"), "srain.cfg", RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet prefs_read_sirc_prefs(SircPrefs *prefs, const char *srv_name){
    SrnRet ret;

    ret = read_sirc_prefs_from_cfg(&builtin_cfg, prefs, srv_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read irc prefs in %s: %s"), "builtin.cfg", RET_MSG(ret));
    }
    read_sirc_prefs_from_cfg(&user_cfg, prefs, srv_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read irc prefs in %s: %s"), "srain.cfg", RET_MSG(ret));
    }

    return SRN_OK;
}

SrnRet prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name,
        const char *chat_name){
    SrnRet ret;
    ret = read_sui_prefs_from_cfg(&builtin_cfg, prefs, srv_name, chat_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read ui prefs in %s: %s"), "builtin.cfg", RET_MSG(ret));
    }
    read_sui_prefs_from_cfg(&user_cfg, prefs, srv_name, chat_name);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Error occurred while read ui prefs in %s: %s"), "srain.cfg", RET_MSG(ret));
    }

    return SRN_OK;
}

static SrnRet prefs_read_file(config_t *cfg, const char *file){
    char *dir;
    const char *version;

    dir = g_path_get_dirname(file);
    config_set_include_dir(cfg, dir);
    g_free(dir);

    if (!config_read_file(cfg, file)){
        return RET_ERR(_("At line %d: %s"),
                config_error_line(cfg),
                config_error_text(cfg));
    }

    /* Verify configure version */
    if (!config_lookup_string(cfg, "version", &version)){
        return RET_ERR(_("No version found"));
    }

    /* TODO:
    if (g_strcmp0(version, PACKAGE_VERSION) != 0){
        return RET_ERR(_("Version doesn't macth");
    }
    */

    return SRN_OK;
}

static SrnRet read_sui_prefs_from_chat(config_setting_t *chat, SuiPrefs *prefs){
    config_setting_lookup_bool_ex(chat, "notify", &prefs->notify);
    config_setting_lookup_bool_ex(chat, "show_topic", &prefs->show_topic);
    config_setting_lookup_bool_ex(chat, "show_avatar", &prefs->show_avatar);
    config_setting_lookup_bool_ex(chat, "show_user_list", &prefs->show_user_list);
    config_setting_lookup_bool_ex(chat, "preview_image", &prefs->preview_image);
    config_setting_lookup_bool_ex(chat, "render_mirc_color", &prefs->render_mirc_color);

    return SRN_OK;
}

static SrnRet read_sui_prefs_from_chat_list(config_setting_t *chat_list,
        SuiPrefs *prefs, const char *chat_name){
    int count;
    SrnRet ret;

    count = config_setting_length(chat_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *chat;

        chat = config_setting_get_elem(chat_list, i);
        if (!chat) break;

        config_setting_lookup_string(chat, "name", &name);
        if (g_strcmp0(chat_name, name) != 0) continue;

        DBG_FR("Read: chat_list.%s, chat_name: %s", name, chat_name);
        ret = read_sui_prefs_from_chat(chat, prefs);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_sui_prefs_from_cfg(config_t *cfg, SuiPrefs *prefs,
        const char *srv_name, const char *chat_name){
    SrnRet ret;

    /* Read server.chat */
    {
        config_setting_t *chat;

        chat = config_lookup(cfg, "server.chat");
        if (chat){
            DBG_FR("Read: server.chat, srv_name: %s, chat_name: %s", srv_name, chat_name);
            ret = read_sui_prefs_from_chat(chat, prefs);
            if (!RET_IS_OK(ret)) return ret;
        }
    }

    /* Read server.chat_list[name = chat_name] */
    if (chat_name){
        config_setting_t *chat_list;

        chat_list = config_lookup(cfg, "server.chat_list");
        if (chat_list){
            ret = read_sui_prefs_from_chat_list(chat_list, prefs, chat_name);
            if (!RET_IS_OK(ret)) return ret;
        }
    }

    if (srv_name){
        config_setting_t *server_list;

        server_list = config_lookup(cfg, "server_list");
        if (server_list){
            int count;

            count = config_setting_length(server_list);
            for (int i = 0; i < count; i++){
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
                    ret =read_sui_prefs_from_chat(chat, prefs);
                    if (!RET_IS_OK(ret)) return ret;
                }

                /* Read server_list.[name = srv_name].chat_list[name = chat_name] */
                if (chat_name){
                    config_setting_t *chat_list;

                    chat_list = config_setting_lookup(server, "server.chat_list");
                    if (chat_list){
                        ret = read_sui_prefs_from_chat_list(chat_list, prefs, chat_name);
                        if (!RET_IS_OK(ret)) return ret;
                    }
                }
            }
        }
    }

    return SRN_OK;
}

static SrnRet read_server_prefs_from_server(config_setting_t *server,
        ServerPrefs *prefs){
    SrnRet ret;

    /* Read server meta info */
    // The name of prefs has been set.
    // config_setting_lookup_string_ex(server, "name", &prefs->name);
    config_setting_lookup_string_ex(server, "host", &prefs->host);
    config_setting_lookup_int(server, "port", &prefs->port);
    config_setting_lookup_string_ex(server, "passwd", &prefs->passwd);
    config_setting_lookup_string_ex(server, "encoding", &prefs->encoding);

    /* Read server.user */
    config_setting_t *user;
    user = config_setting_lookup(server, "user");
    if (user){
        config_setting_lookup_string_ex(user, "nickname", &prefs->nickname);
        config_setting_lookup_string_ex(user, "username", &prefs->username);
        config_setting_lookup_string_ex(user, "realname", &prefs->realname);
    }

    /* Read server.default_messages */
    config_setting_t *default_messages;
    default_messages = config_setting_lookup(server, "default_messages");
    if (default_messages){
        config_setting_lookup_string_ex(default_messages, "part", &prefs->part_message);
        config_setting_lookup_string_ex(default_messages, "kick", &prefs->kick_message);
        config_setting_lookup_string_ex(default_messages, "away", &prefs->away_message);
        config_setting_lookup_string_ex(default_messages, "quit", &prefs->quit_message);
    }

    /* Read server.irc */
    config_setting_t *irc;
    irc = config_setting_lookup(server, "irc");
    if (irc){
        ret = read_sirc_prefs_from_irc(irc, prefs->irc);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_server_prefs_from_server_list(config_setting_t *server_list,
        ServerPrefs *prefs){
    int count;
    SrnRet ret;

    count = config_setting_length(server_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *server;

        server = config_setting_get_elem(server_list, i);
        if (!server) break;

        config_setting_lookup_string(server, "name", &name);
        if (g_strcmp0(prefs->name, name) != 0) continue;

        DBG_FR("Read: server_list.[name = %s], srv_name: %s", name, prefs->name);
        ret = read_server_prefs_from_server(server, prefs);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs){
    SrnRet ret;
    config_setting_t *server;

    /* Read server */
    server = config_lookup(cfg, "server");
    if (server){
        DBG_FR("Read: server, srv_name: %s", prefs->name);
        ret = read_server_prefs_from_server(server, prefs);
        if (!RET_IS_OK(ret)) return ret;
    }

    /* Read server_list[name = prefs->name] */
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server_list");
    if (server_list){
        ret = read_server_prefs_from_server_list(server_list, prefs);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

static SrnRet read_sirc_prefs_from_irc(config_setting_t *irc, SircPrefs *prefs){
    config_setting_lookup_bool_ex(irc, "tls", &prefs->tls);
    config_setting_lookup_bool_ex(irc, "tls_not_verify", &prefs->tls_not_verify);

    return SRN_OK;
}

static SrnRet read_sirc_prefs_from_server_list(config_setting_t *server_list,
        SircPrefs *prefs, const char *srv_name){
    int count;

    count = config_setting_length(server_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *server;
        config_setting_t *irc;

        server = config_setting_get_elem(server_list, i);
        if (!server) break;

        config_setting_lookup_string(server, "name", &name);
        if (g_strcmp0(srv_name, name) != 0) continue;

        irc = config_setting_lookup(server, "irc");
        if (!irc) break;

        DBG_FR("Read: server_list.[name = %s].irc, srv_name: %s", name, srv_name);
        read_sirc_prefs_from_irc(server, prefs);
    }

    return SRN_OK;
}

static SrnRet read_sirc_prefs_from_cfg(config_t *cfg, SircPrefs *prefs,
        const char *srv_name){
    SrnRet ret;
    config_setting_t *irc;

    /* Read server.irc */
    irc = config_lookup(cfg, "server.irc");
    if (irc){
        DBG_FR("Read: server.irc, srv_name: %s", srv_name);
        ret = read_sirc_prefs_from_irc(irc, prefs);
        if (!RET_IS_OK(ret)) return ret;
    }

    /* Read server_list[name = srv_name] */
    if (srv_name){
        config_setting_t *server_list;

        server_list = config_lookup(cfg, "server_list");
        if (server_list){
            ret = read_sirc_prefs_from_server_list(server_list, prefs, srv_name);
            if (!RET_IS_OK(ret)) return ret;
        }
    }

    return SRN_OK;
}

static SrnRet read_server_prefs_list_from_cfg(config_t *cfg){
    SrnRet ret;
    int count;
    ServerPrefs *prefs;
    config_setting_t *server;
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server_list");

    if (server_list){
        count = config_setting_length(server_list);
        for (int i = 0; i < count; i++){
            const char *name = NULL;
            server = config_setting_get_elem(server_list, i);
            if (!server) break;

            if (config_setting_lookup_string(server, "name", &name) != CONFIG_TRUE) {
                return RET_ERR(_("Server[%d] in server_list doesn't have a name"), i);
            }

            prefs = server_prefs_new(name);
            if (!prefs){
                return RET_ERR(_("Server already exist: %s"), name);
            }

            ret = prefs_read_server_prefs(prefs);
            if (!RET_IS_OK(ret)){
                g_free(prefs);
                return ret;
            }
        }
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

static SrnRet read_log_prefs_from_cfg(config_t *cfg, LogPrefs *prefs){
    SrnRet ret;
    config_setting_t *log;

    log = config_lookup(cfg, "log");

    if (log){
        config_setting_lookup_bool_ex(log, "prompt_color", &prefs->prompt_color);
        config_setting_lookup_bool_ex(log, "prompt_file", &prefs->prompt_file);
        config_setting_lookup_bool_ex(log, "prompt_function", &prefs->prompt_function);
        config_setting_lookup_bool_ex(log, "prompt_line", &prefs->prompt_line);

        ret = read_log_targets_from_log(log, "debug_targets", &prefs->debug_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "info_targets", &prefs->info_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "warn_targets", &prefs->warn_targets);
        if (!RET_IS_OK(ret)) return ret;
        ret = read_log_targets_from_log(log, "error_targets", &prefs->error_targets);
        if (!RET_IS_OK(ret)) return ret;
    }

    return SRN_OK;
}

/* The "bool" in libconfig is actually an integer, transform it to fit bool in "stdbool.h". */
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

/* This function will allocate a new string, you should free it after used. */
static int config_lookup_string_ex(const config_t *config, const char *path, char **value){
    int ret;
    const char *constval = NULL;

    ret = config_lookup_string(config, path, &constval);

    if (constval){
        *value = g_strdup(constval);
    }

    return ret;
}

/* This function will allocate a new string, you should free it after used. */
static int config_setting_lookup_string_ex(const config_setting_t *config,
        const char *name, char **value){
    int ret;
    const char *constval = NULL;

    ret = config_setting_lookup_string(config, name, &constval);

    if (constval){
        *value = g_strdup(constval);
    }

    return ret;
}
