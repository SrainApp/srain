/**
 * @file prefs.c
 * @brief libconfig warpper for srain
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-05-14
 */

#define __LOG_ON

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

static const char *prefs_read_file(config_t *cfg, const char *file);

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

    return NULL;
}
