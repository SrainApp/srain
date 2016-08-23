/**
 * @file file_helper.c
 * @brief Get application data files' path
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-12
 */

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

    path = g_build_filename(".", fname, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }
    WARN_FR("'%s' not found", path);
    g_free(path);

    path = g_build_filename(g_get_user_config_dir(), PACKAGE,
            fname, NULL);

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

    WARN_FR("'%s' not found", path);
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
