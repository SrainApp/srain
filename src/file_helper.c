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
 * @file file_helper.c
 * @brief Get application data files' path
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-06-12
 */

#include <stdio.h>
#include <sys/stat.h>
#include <glib.h>
#include <errno.h>

#include "meta.h"
#include "log.h"

/**
 * @brief get_theme_file
 *
 * @param fname
 *
 * @return NULL or path to the theme file, must be freed.
 */
char *get_theme_file(const char *fname){
    char *path;

    path = g_build_filename(PACKAGE_DATA_DIR, "share",
            PACKAGE, "themes", fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

/**
 * @brief get_pixmap_file 
 *
 * @param fname
 *
 * @return NULL or path to the pixmap file, must be freed.
 */
char *get_pixmap_file(const char *fname){
    char *path;

    path = g_build_filename(PACKAGE_DATA_DIR, "share",
            PACKAGE, "pixmaps", fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

/**
 * @brief get_plugin_file
 *
 * @param fname
 *
 * @return NULL or path to the pixmap file, must be freed.
 */
char *get_plugin_file(const char *fname){
    char *path;

    path = g_build_filename(PACKAGE_DATA_DIR, "share",
            PACKAGE, "plugins", fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

/**
 * @brief get_config_file
 *
 * @param fname
 *
 * @return NULL or path to the theme file, must be freed.
 */
char *get_config_file(const char *fname){
    char *path;

    path = g_build_filename(g_get_user_config_dir(), PACKAGE,
            fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

char *get_system_config_file(const char *fname){
    char *path;

    path = g_build_filename(PACKAGE_CONFIG_DIR, PACKAGE, fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);

    return NULL;
}

char *get_avatar_file(const char *fname){
    char *path;

    path = g_build_filename(g_get_user_cache_dir(),
            PACKAGE, "avatars", fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    // WARN_FR("'%s' not found", path);
    g_free(path);

    return NULL;
}

char *create_log_file(const char *srv_name, const char *fname){
    int res;
    char *path;

    path = g_build_filename(g_get_user_data_dir(),
            PACKAGE, "logs", srv_name, fname, NULL);

    if (!g_file_test(path, G_FILE_TEST_EXISTS)){
        char *dir;

        /* Create if dir non-exist */
        dir = g_build_filename(g_get_user_data_dir(),
                PACKAGE, "logs", srv_name, NULL);
        res = mkdir(dir, 0700);
        if (res == -1) {
            if (errno != EEXIST){
                ERR_FR("Failed to create directory '%s', errno %d", dir, errno);
                g_free(dir);
                return NULL;
            }
        }
        g_free(dir);
    }

    return path;
}


/**
 * @brief Create directories and config files if no exist
 *          - $XDG_CONFIG_HOME/srain/
 *          - $XDG_CONFIG_HOME/srain/srainrc
 *          - $XDG_CACHE_HOME/srain/
 *          - $XDG_CACHE_HOME/srain/avatars
 *          - $XDG_DATA_HOME/srain/logs
 *
 * @return 0 if all required files are created or already existent
 */
int create_user_file(){
    int res;
    FILE *fp;
    char *congif_dir;
    char *cache_dir;
    char *rc_file;
    char *data_dir;
    char *logs_dir;

    congif_dir = g_build_filename(g_get_user_config_dir(), "srain", NULL);
    res = mkdir(congif_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("Failed to create directory '%s', errno %d", congif_dir, errno);
            return errno;
        }
    }
    g_free(congif_dir);
    congif_dir = NULL;

    rc_file = g_build_filename(g_get_user_config_dir(), "srain", "srainrc", NULL);
    fp = fopen(rc_file, "r");
    if (!fp){
        fp = fopen(rc_file, "w");
        if (!fp){
            ERR_FR("Failed to create file '%s', errno %d", rc_file, errno);
            exit(errno);
        }
    }
    g_free(rc_file);
    rc_file = NULL;

    cache_dir = g_build_filename(g_get_user_cache_dir(), "srain", NULL);
    res = mkdir(cache_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("Failed to create directory '%s', errno %d", cache_dir, errno);
            return errno;
        }
    }
    g_free(cache_dir);
    cache_dir = NULL;

    cache_dir = g_build_filename(g_get_user_cache_dir(), "srain", "avatars", NULL);
    res = mkdir(cache_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("Failed to create directory '%s', errno %d", cache_dir, errno);
            return errno;
        }
    }
    g_free(cache_dir);
    cache_dir = NULL;

    data_dir = g_build_filename(g_get_user_data_dir(), "srain", NULL);
    LOG_FR("%s", data_dir);
    res = mkdir(data_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("Failed to create directory '%s', errno %d", data_dir, errno);
            return errno;
        }
    }
    g_free(data_dir);
    data_dir = NULL;

    logs_dir = g_build_filename(g_get_user_data_dir(), "srain", "logs", NULL);
    res = mkdir(logs_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("Failed to create directory '%s', errno %d", logs_dir, errno);
            return errno;
        }
    }
    g_free(logs_dir);
    logs_dir = NULL;

    return 0;
}
