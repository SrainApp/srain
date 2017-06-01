/**
 * @file prefs.c
 * @brief Libconfig based configure file reader
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-05-14
 *
 * Prefs priority:
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

static char *prefs_read_file(config_t *cfg, const char *file);

static void read_sirc_prefs_from_irc(config_setting_t *irc, SircPrefs *prefs);
static void read_sirc_prefs_from_server_list(config_setting_t *server_list, SircPrefs *prefs, const char *srv_name);
static void read_sirc_prefs_from_cfg(config_t *cfg, SircPrefs *prefs, const char *srv_name);

static void read_server_prefs_from_server(config_setting_t *server, ServerPrefs *prefs);
static void read_server_prefs_from_server_list(config_setting_t *server_list, ServerPrefs *prefs, const char *srv_name);
static void read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs, const char *srv_name);

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
    config_lookup_string(&builtin_cfg, "application.theme", &prefs->theme);
    config_lookup_string(&user_cfg, "application.theme", &prefs->theme);

    return NULL;
}

char *prefs_read_server_list(GList **server_list){
    // TODO
    return NULL;
}

char* prefs_read_server_prefs(ServerPrefs *prefs, const char *srv_name){
    read_server_prefs_from_cfg(&builtin_cfg, prefs, srv_name);
    read_server_prefs_from_cfg(&user_cfg, prefs, srv_name);

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
    config_setting_lookup_bool(chat, "notify", (int *)&prefs->notify);
    config_setting_lookup_bool(chat, "show_topic", (int *)&prefs->show_topic);
    config_setting_lookup_bool(chat, "show_avatar", (int *)&prefs->show_avatar);
    config_setting_lookup_bool(chat, "show_user_list", (int *)&prefs->show_user_list);
    config_setting_lookup_bool(chat, "preview_image", (int *)&prefs->preview_image);
    config_setting_lookup_bool(chat, "enable_log", (int *)&prefs->enable_log);
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

        DBG_FR("Read: chat_list.%s", name);
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
            DBG_FR("Read: server.chat")
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
                    DBG_FR("Read server_list.[name = %s].chat", name);
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
    config_setting_lookup_string(server, "name", &prefs->name);
    config_setting_lookup_string(server, "host", &prefs->host);
    config_setting_lookup_int(server, "port", &prefs->port);
    config_setting_lookup_string(server, "passwd", &prefs->passwd);
    config_setting_lookup_string(server, "encoding", &prefs->encoding);

    /* Read server.user */
    config_setting_lookup_string(server, "user.nickname", &prefs->nickname);
    config_setting_lookup_string(server, "user.username", &prefs->username);
    config_setting_lookup_string(server, "user.realname", &prefs->realname);

    /* Read server.default_messages */
    config_setting_lookup_string(server, "default_messages.part", &prefs->part_message);
    config_setting_lookup_string(server, "default_messages.kick", &prefs->kick_message);
    config_setting_lookup_string(server, "default_messages.away", &prefs->away_message);
    config_setting_lookup_string(server, "default_messages.quit", &prefs->quit_message);
}

static void read_server_prefs_from_server_list(config_setting_t *server_list,
        ServerPrefs *prefs, const char *srv_name){
    int count;

    count = config_setting_length(server_list);
    for (int i = 0; i < count; i++){
        const char *name = NULL;
        config_setting_t *server;

        server = config_setting_get_elem(server_list, i);
        if (!server) break;

        config_setting_lookup_string(server, "name", &name);
        if (g_strcmp0(srv_name, name) != 0) continue;

        DBG_FR("Read: server_list.[name = %s]", name);
        read_server_prefs_from_server(server, prefs);
    }
}

static void read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs,
        const char *srv_name){
    config_setting_t *server;

    /* Read server */
    server = config_lookup(cfg, "server");
    if (server){
        DBG_FR("Read: server");
        read_server_prefs_from_server(server, prefs);
    }

    /* Read server_list[name = srv_name] */
    if (srv_name){
        config_setting_t *server_list;

        server_list = config_lookup(cfg, "server_list");
        if (server_list){
            read_server_prefs_from_server_list(server_list, prefs, srv_name);
        }
    }
}

static void read_sirc_prefs_from_irc(config_setting_t *irc, SircPrefs *prefs){
    config_setting_lookup_bool(irc, "auto_reconnect", (int *)&prefs->auto_reconnect);
    config_setting_lookup_bool(irc, "use_ssl", (int *)&prefs->use_ssl);
    config_setting_lookup_bool(irc, "verify_ssl_cert", (int *)&prefs->verify_ssl_cert);
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

        DBG_FR("Read: server_list.[name = %s].irc", name);
        read_sirc_prefs_from_irc(server, prefs);
    }
}

static void read_sirc_prefs_from_cfg(config_t *cfg, SircPrefs *prefs,
        const char *srv_name){
    config_setting_t *irc;

    /* Read server.irc */
    irc = config_lookup(cfg, "server.irc");
    if (irc){
        DBG_FR("Read: server.irc");
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
