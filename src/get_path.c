/**
 * @file get_path.c
 * @brief Get application data files' path
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-12
 */
#include <glib.h>

#include "meta.h"
#include "log.h"

/**
 * @brief get_theme_path
 *
 * @param filename
 *
 * @return NULL or path to the theme file, must be freed.
 */
char *get_theme_path(const char *filename){
    char *path;

    path = g_build_filename(PACKAGE_DATA_DIR, "share",
            PACKAGE, "themes", filename, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

/**
 * @brief get_pixmap_path 
 *
 * @param filename
 *
 * @return NULL or path to the pixmap file, must be freed.
 */
char *get_pixmap_path(const char *filename){
    char *path;

    path = g_build_filename(PACKAGE_DATA_DIR, "share",
            PACKAGE, "pixmaps", filename, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

/**
 * @brief get_plugin_path
 *
 * @param filename
 *
 * @return NULL or path to the pixmap file, must be freed.
 */
char *get_plugin_path(const char *filename){
    char *path;

    path = g_build_filename(PACKAGE_DATA_DIR, "share",
            PACKAGE, "plugins", filename, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

/**
 * @brief get_config_path
 *
 * @param filename
 *
 * @return NULL or path to the theme file, must be freed.
 */
char *get_config_path(const char *filename){
    char *path;

    path = g_build_filename(".", filename, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }
    WARN_FR("'%s' not found", path);
    g_free(path);

    path = g_build_filename(g_get_user_config_dir(), PACKAGE,
            filename, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);
    return NULL;
}

char *get_avatar_path(const char *filename){
    char *path;

    path = g_build_filename(g_get_user_cache_dir(),
            PACKAGE, "avatars", filename, NULL);

    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return path;
    }

    WARN_FR("'%s' not found", path);
    g_free(path);

    return NULL;
}
