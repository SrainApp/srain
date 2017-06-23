/**
 * @file prefs.c
 * @brief Libconfig based configure file reader
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-05-14
 *
 * It read config from the sysetm-wide config file "builtin.cfg" and user-wide
 * config file "srain.cfg". Path of system-wide config file is obtained by
 * ``get_system_config_file()``, and the path of user-wide config file is obtained
 * by ``get_config_file()``.
 *
 * The structure of these two files is same, but the priority of user-wide
 * config file is bigger than system-wide config file.
 *
 * For the syntax of config file, refer to:
 *   http://www.hyperrealm.com/libconfig/libconfig_manual.html#Configuration-Files
 *
 * [TODO] Prefs priority:
 *      builtin_cfg < user_cfg
 *      server < server_list
 *      chat < chat_list
 */

#define __LOG_ON
#define __DBG_ON

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

static char* prefs_read_file(config_t *cfg, const char *file);

static int config_lookup_string_ex(const config_t *config, const char *path, char **value);
static int config_setting_lookup_bool_ex(const config_setting_t *config, const char *name, bool *value);
static int config_setting_lookup_string_ex(const config_setting_t *config, const char *name, char **value);

static void read_sirc_prefs_from_irc(config_setting_t *irc, SircPrefs *prefs);
static void read_sirc_prefs_from_server_list(config_setting_t *server_list, SircPrefs *prefs, const char *srv_name);
static void read_sirc_prefs_from_cfg(config_t *cfg, SircPrefs *prefs, const char *srv_name);

static void read_server_prefs_from_server(config_setting_t *server, ServerPrefs *prefs);
static void read_server_prefs_from_server_list(config_setting_t *server_list, ServerPrefs *prefs);
static void read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs);
static void read_server_prefs_list_from_cfg(config_t *cfg);

static void read_sui_prefs_from_chat(config_setting_t *chat, SuiPrefs *prefs);
static void read_sui_prefs_from_chat_list(config_setting_t *chat_list, SuiPrefs *prefs, const char *chat_name);
static void read_sui_prefs_from_cfg(config_t *cfg, SuiPrefs *prefs, const char *srv_name, const char *chat_name);

void prefs_init(){
    config_init(&user_cfg);
    config_init(&builtin_cfg);
};

void prefs_finalize(){
    config_destroy(&user_cfg);
    config_destroy(&builtin_cfg);
};

char* prefs_read(){
    char *path;
    char *errmsg;

    path = get_system_config_file("builtin.cfg");
    if (!path){
        return g_strdup(_("File 'builtin.cfg' not found."));
    }

    errmsg = prefs_read_file(&builtin_cfg, path);
    g_free(path);

    if (errmsg){
        return errmsg;
    }

    path = get_config_file("srain.cfg");
    if (!path){
        // return g_strdup(_("File 'builtin.cfg' not found."));
        return NULL;
    }

    errmsg = prefs_read_file(&user_cfg, path);
    g_free(path);

    if (errmsg){
        return errmsg;
    }

    return NULL;
}

char* prefs_read_sui_app_prefs(SuiAppPrefs *prefs){
    // TODO: read_sui_app_prefs_from_cfg()
    config_lookup_string_ex(&builtin_cfg, "application.theme", &prefs->theme);
    config_lookup_string_ex(&user_cfg, "application.theme", &prefs->theme);

    return NULL;
}

char *prefs_read_server_prefs_list(){
    read_server_prefs_list_from_cfg(&builtin_cfg);
    read_server_prefs_list_from_cfg(&user_cfg);

    return NULL;
}

char* prefs_read_server_prefs(ServerPrefs *prefs){
    read_server_prefs_from_cfg(&builtin_cfg, prefs);
    read_server_prefs_from_cfg(&user_cfg, prefs);

    return NULL;
}

char* prefs_read_sirc_prefs(SircPrefs *prefs, const char *srv_name){
    read_sirc_prefs_from_cfg(&builtin_cfg, prefs, srv_name);
    read_sirc_prefs_from_cfg(&user_cfg, prefs, srv_name);

    return NULL;
}

char *prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name,
        const char *chat_name){
    read_sui_prefs_from_cfg(&builtin_cfg, prefs, srv_name, chat_name);
    read_sui_prefs_from_cfg(&user_cfg, prefs, srv_name, chat_name);

    return NULL;
}

static char *prefs_read_file(config_t *cfg, const char *file){
    char *dir;
    const char *version;

    dir = g_path_get_dirname(file);
    config_set_include_dir(cfg, dir);
    g_free(dir);

    if (!config_read_file(cfg, file)){
        return g_strdup_printf(_("Error found in %s line %d: %s"),
                config_error_file(cfg),
                config_error_line(cfg),
                config_error_text(cfg));
    }

    /* Verify configure version */
    if (!config_lookup_string(cfg, "version", &version)){
        return g_strdup_printf(_("No version found in config'%s'"), file);
    }

    /* TODO:
    if (strcmp(version, PACKAGE_VERSION) != 0){
        return g_strdup_printf(_("Version in config file '%s' is not macth"), file);
    }
    */

    return NULL;
}

static void read_sui_prefs_from_chat(config_setting_t *chat, SuiPrefs *prefs){
    config_setting_lookup_bool_ex(chat, "notify", &prefs->notify);
    config_setting_lookup_bool_ex(chat, "show_topic", &prefs->show_topic);
    config_setting_lookup_bool_ex(chat, "show_avatar", &prefs->show_avatar);
    config_setting_lookup_bool_ex(chat, "show_user_list", &prefs->show_user_list);
    config_setting_lookup_bool_ex(chat, "preview_image", &prefs->preview_image);
}

static void read_sui_prefs_from_chat_list(config_setting_t *chat_list,
        SuiPrefs *prefs, const char *chat_name){
    int count;

    count = config_setting_length(chat_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *chat;

        chat = config_setting_get_elem(chat_list, i);
        if (!chat) break;

        config_setting_lookup_string(chat, "name", &name);
        if (g_strcmp0(chat_name, name) != 0) continue;

        DBG_FR("Read: chat_list.%s, chat_name: %s", name, chat_name);
        read_sui_prefs_from_chat(chat, prefs);
    }
}

static void read_sui_prefs_from_cfg(config_t *cfg, SuiPrefs *prefs,
        const char *srv_name, const char *chat_name){

    /* Read server.chat */
    {
        config_setting_t *chat;

        chat = config_lookup(cfg, "server.chat");
        if (chat){
            DBG_FR("Read: server.chat, srv_name: %s, chat_name: %s", srv_name, chat_name);
            read_sui_prefs_from_chat(chat, prefs);
        }
    }

    /* Read server.chat_list[name = chat_name] */
    if (chat_name){
        config_setting_t *chat_list;

        chat_list = config_lookup(cfg, "server.chat_list");
        if (chat_list){
            read_sui_prefs_from_chat_list(chat_list, prefs, chat_name);
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
                    read_sui_prefs_from_chat(chat, prefs);
                }

                /* Read server_list.[name = srv_name].chat_list[name = chat_name] */
                if (chat_name){
                    config_setting_t *chat_list;

                    chat_list = config_setting_lookup(server, "server.chat_list");
                    if (chat_list){
                        read_sui_prefs_from_chat_list(chat_list, prefs, chat_name);
                    }
                }
            }
        }
    }
}

static void read_server_prefs_from_server(config_setting_t *server,
        ServerPrefs *prefs){
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
        read_sirc_prefs_from_irc(irc, prefs->irc);
    }
}

static void read_server_prefs_from_server_list(config_setting_t *server_list,
        ServerPrefs *prefs){
    int count;

    count = config_setting_length(server_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *server;

        server = config_setting_get_elem(server_list, i);
        if (!server) break;

        config_setting_lookup_string(server, "name", &name);
        if (g_strcmp0(prefs->name, name) != 0) continue;

        DBG_FR("Read: server_list.[name = %s], srv_name: %s", name, prefs->name);
        read_server_prefs_from_server(server, prefs);
    }
}

static void read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs){
    config_setting_t *server;

    /* Read server */
    server = config_lookup(cfg, "server");
    if (server){
        DBG_FR("Read: server, srv_name: %s", prefs->name);
        read_server_prefs_from_server(server, prefs);
    }

    /* Read server_list[name = prefs->name] */
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server_list");
    if (server_list){
        read_server_prefs_from_server_list(server_list, prefs);
    }
}

static void read_sirc_prefs_from_irc(config_setting_t *irc, SircPrefs *prefs){
    config_setting_lookup_bool_ex(irc, "auto_reconnect", &prefs->auto_reconnect);
    config_setting_lookup_bool_ex(irc, "use_ssl", &prefs->use_ssl);
    config_setting_lookup_bool_ex(irc, "verify_ssl_cert", &prefs->verify_ssl_cert);
}

static void read_sirc_prefs_from_server_list(config_setting_t *server_list,
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
}

static void read_sirc_prefs_from_cfg(config_t *cfg, SircPrefs *prefs,
        const char *srv_name){
    config_setting_t *irc;

    /* Read server.irc */
    irc = config_lookup(cfg, "server.irc");
    if (irc){
        DBG_FR("Read: server.irc, srv_name: %s", srv_name);
        read_sirc_prefs_from_irc(irc, prefs);
    }

    /* Read server_list[name = srv_name] */
    if (srv_name){
        config_setting_t *server_list;

        server_list = config_lookup(cfg, "server_list");
        if (server_list){
            read_sirc_prefs_from_server_list(server_list, prefs, srv_name);
        }
    }
}

static void read_server_prefs_list_from_cfg(config_t *cfg){
    int count;
    ServerPrefs *prefs;
    config_setting_t *server;
    config_setting_t *server_list;

    server_list = config_lookup(cfg, "server_list");

    if (server_list){
        count = config_setting_length(server_list);
        for (int i = 0; i < count; i++){
            char *errmsg = NULL;
            const char *name = NULL;
            server = config_setting_get_elem(server_list, i);
            if (!server) break;

            config_setting_lookup_string(server, "name", &name);
            if (!name) break;

            prefs = server_prefs_new(name);
            if (!prefs){
                ERR_FR("Failed to create ServerPrefs '%s'", name);
            }

            errmsg = prefs_read_server_prefs(prefs);
            if (errmsg){
                g_free(errmsg);
                g_free(prefs);
                break;
            }
        }
    }
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
