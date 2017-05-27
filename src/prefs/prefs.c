/**
 * @file prefs.c
 * @brief libconfig warpper for srain
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-05-14
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

char *prefs_read_server_prefs(ServerPrefs *prefs, const char *srv_name);
char *prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name,
        const char *chat_name);

static const char *prefs_read_file(config_t *cfg, const char *file);

static void read_sirc_prefs_from_irc(config_t *cfg, SircPrefs *prefs);

static void read_server_prefs_from_server(config_setting_t *server,
        ServerPrefs *prefs);
static void read_server_prefs_from_server_list(config_setting_t *server_list,
        ServerPrefs *prefs, const char *srv_name);
static void read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs,
        const char *srv_name);

static void read_sui_prefs_from_chat(config_setting_t *chat, SuiPrefs *prefs);
static void read_sui_prefs_from_chat_list(config_setting_t *chat_list,
        SuiPrefs *prefs, const char *chat_name);
static void read_sui_prefs_from_cfg(config_t *cfg, SuiPrefs *prefs,
        const char *srv_name, const char *chat_name);

void prefs_init(){
    config_init(&user_cfg);
    config_init(&builtin_cfg);
};

const char* prefs_read(){
    char *path;
    const char *errmsg;

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

void prefs_finalize(){
    config_destroy(&user_cfg);
    config_destroy(&builtin_cfg);
};

static const char *prefs_read_file(config_t *cfg, const char *file){
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
    SuiPrefs *prefs = g_malloc0(sizeof(SuiPrefs));

    prefs_read_sui_prefs(prefs, NULL, NULL);
    prefs_read_sui_prefs(prefs, NULL, "#srain");

    return NULL;
}

static void read_server_prefs_from_server(config_setting_t *server,
        ServerPrefs *prefs){
    config_setting_t *irc;

    config_setting_lookup_string(server, "nickname", &prefs->nickname);

    /* Read server.irc */
    irc = config_setting_lookup(server, "irc");
    if (irc){
        // read_sirc_prefs_from_irc(irc, prefs->irc);
    }
}

static void read_sui_prefs_from_chat(config_setting_t *chat, SuiPrefs *prefs){
    /* TODO: read more element */
    // config_setting_lookup_bool(chat, "show_topic", &((int)prefs->show_topic));
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

        DBG_FR("Read: chat_list.%s", name);

        if (g_strcmp0(chat_name, name) == 0){
            read_sui_prefs_from_chat(chat, prefs);
            break;
        }
    }
}

static void read_sui_prefs_from_cfg(config_t *cfg, SuiPrefs *prefs,
        const char *srv_name, const char *chat_name){

    /* Read server.chat */
    {
        config_setting_t *chat;

        chat = config_lookup(cfg, "server.chat");
        if (chat){
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

                server = config_setting_get_elem(server_list, i);
                if (!server) break;

                config_setting_lookup_string(server, "name", &name);

                if (g_strcmp0(srv_name, name) != 0){
                    continue;
                }

                DBG_FR("Config server[name = %s] found", name);

                config_setting_t *chat;

                /* Read server_list.[name = srv_name].chat */
                chat = config_setting_lookup(server, "chat");
                if (chat){
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

char* prefs_read_sui_app_prefs(SuiAppPrefs *prefs){
    return NULL;
}

char* prefs_read_server_prefs(ServerPrefs *prefs, const char *srv_name){
    read_server_prefs_from_cfg(&builtin_cfg, prefs, srv_name);
    read_server_prefs_from_cfg(&user_cfg, prefs, srv_name);

    return NULL;
}

char *prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name,
        const char *chat_name){
    // g_return_val_if_fail(prefs, );

    /* builtin < user
     * default prefs < server.chat < server.chat_list < server_list.chat < server_list.chat_list
     */
    read_sui_prefs_from_cfg(&builtin_cfg, prefs, srv_name, chat_name);
    read_sui_prefs_from_cfg(&user_cfg, prefs, srv_name, chat_name);

    return NULL;
}

char *prefs_read_server_list(GList **server_list){
    return NULL;
}

static void read_server_prefs_from_cfg(config_t *cfg, ServerPrefs *prefs,
        const char *srv_name){
    config_setting_t *server;

    /* Read server */
    server = config_lookup(cfg, "server");
    if (server){
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

        DBG_FR("Read: server_list.[name = %s]", name);

        if (g_strcmp0(srv_name, name) == 0){
            read_server_prefs_from_server(server, prefs);
            break;
        }
    }
}

static void read_sirc_prefs_from_irc(config_t *cfg, SircPrefs *prefs){

}
