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
 * @file manager.c
 * @brief Configuration manager for Srain, use GSettings backend
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-05-14
 */

#include <glib.h>
#include <string.h>

#include "config/config.h"
#include "./password.h"

#include "log.h"
#include "meta.h"
#include "path.h"
#include "i18n.h"
#include "version.h"

static void srn_settings_struct_init(SrnSettingsStruct *cfg);
static void srn_settings_struct_destroy(SrnSettingsStruct *cfg);
static SrnRet load_config(SrnConfigManager *mgr, SrnSettingsStruct *cfg, const char *file);

SrnConfigManager *srn_config_manager_new(SrnVersion *ver){
    SrnConfigManager *mgr;

    mgr = g_malloc0(sizeof(SrnConfigManager));
    mgr->ver = ver;
    srn_settings_struct_init(&mgr->gs_user_cfg);
    srn_settings_struct_init(&mgr->gs_system_cfg);
    srn_config_manager_init_secret_schema(mgr);

    return mgr;
}

void srn_config_manager_free(SrnConfigManager *mgr){
    srn_settings_struct_destroy(&mgr->gs_user_cfg);
    srn_settings_struct_destroy(&mgr->gs_system_cfg);
    g_free(mgr);
}

SrnRet srn_config_manager_load_system_config(SrnConfigManager *mgr,
        const char *file){
    SrnRet ret;

    ret = load_config(mgr, &mgr->gs_system_cfg, file);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Failed to read system configuration file: %1$s"),
                RET_MSG(ret));
    }
    return SRN_OK;
}

SrnRet srn_config_manager_load_user_config(SrnConfigManager *mgr,
        const char *file){
    SrnRet ret;

    ret = load_config(mgr, &mgr->gs_user_cfg, file);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Failed to read user configuration file: %1$s"),
                RET_MSG(ret));
    }
    return SRN_OK;
}

static SrnRet load_config(SrnConfigManager *mgr, SrnSettingsStruct *cfg, const char *file){
    // const char *rawver;
    SrnVersion *ver;
    SrnRet ret;

    ver = NULL;
    /*
    dir = g_path_get_dirname(file);
    config_set_include_dir(cfg, dir);

    if (!config_read_file(cfg, file)){
        ret = RET_ERR(_("At line %1$d: %2$s"),
                config_error_line(cfg),
                config_error_text(cfg));
        goto FIN;
    }*/

    srn_settings_struct_destroy(cfg);
    cfg->gs_backend = g_keyfile_settings_backend_new(file,
                                                     PACKAGE_GSCHEMA_ROOT_PATH,
                                                     PACKAGE_GSCHEMA_ROOT_GROUP);
    cfg->gs_cfg = g_settings_new_with_backend(PACKAGE_GSCHEMA_SETTINGS_ID,
                                              cfg->gs_backend);

    /* Verify configure version */
    // if (!config_lookup_string(cfg, "version", &rawver)){
    //     ret = RET_ERR(_("No version found in configuration file: %1$s"), file);
    //     goto FIN;
    // }
    // ver = srn_version_new(rawver);
    // ret = srn_version_parse(ver);
    // if (!RET_IS_OK(ret)){
    //     ret = RET_ERR(_("Failed to parse version: %s"), RET_MSG(ret));
    //     goto FIN;
    // }
    // LOG_FR("Configuration file version: %d.%d.%d", ver->major, ver->minor, ver->micro);

    /*
    if (mgr->ver->major > ver->major){
        ret = RET_ERR(_("Configuration file version: %1$d, application version: %2$d, "
                    "Please migrate your profile to new version"),
                ver->major,
                mgr->ver->major);
        goto FIN;
    }
    */

    ret = SRN_OK;
//FIN:
    if (ver){
        srn_version_free(ver);
    }
    return ret;
}

static void srn_settings_struct_init(SrnSettingsStruct *cfg){
    cfg->gs_backend = g_null_settings_backend_new();
    cfg->gs_cfg = g_settings_new_with_backend(PACKAGE_GSCHEMA_SETTINGS_ID,
                                                   cfg->gs_backend);
}

static void srn_settings_struct_destroy(SrnSettingsStruct *cfg){
    g_clear_object(&cfg->gs_cfg);
    g_clear_object(&cfg->gs_backend);
}
