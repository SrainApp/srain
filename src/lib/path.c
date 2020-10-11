/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
 * Copyright (C) 2020 Fei Li <lifeibiren@gmail.com>
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
 * @file path.c
 * @brief Get application data files' path
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2018-12-15
 */


#include <sys/stat.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <errno.h>

#include "meta.h"
#include "log.h"
#include "i18n.h"
#include "path.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <Shlwapi.h>
#include <io.h>

#elif defined __APPLE__
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#elif defined __linux__
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

#define PROC_SELF_EXE "/proc/self/exe"

#elif defined __sun

#define PROC_SELF_EXE "/proc/self/path/a.out"

#elif defined __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>

#endif

#define DEFAULT_FILE_MODE   (S_IRUSR | S_IWUSR)
#define DEFAULT_DIR_MODE    (S_IRWXU)

static SrnRet create_file_if_not_exist(const char *path);
static SrnRet create_dir_if_not_exist(const char *path);

static void strfreev(char **prefix){
    if (*prefix) {
        g_free(*prefix);
        strfreev(prefix + 1);
    }
}

static char *srn_find_file_in_prefix(char *prefix[], const char *name){
    char *path;

    for (;;){
        if (*prefix == NULL){
            break;
        }

        path = g_build_filename(*prefix, name, NULL);

        if (g_file_test(path, G_FILE_TEST_EXISTS)){
            return path;
        }

        g_free(path);

        ++prefix;
    }

    return NULL;
}

static char *srn_try_to_find_data_file(const char *name){
    char *path;
    char *prefix[] = {
        g_build_filename(PACKAGE_DATA_DIR, PACKAGE, NULL),
        g_build_filename(srn_get_executable_dir(), "share", PACKAGE, NULL),
        g_build_filename(srn_get_executable_dir(), "..", "share", PACKAGE, NULL),
        srn_get_executable_dir(),
        NULL
    };

    path = srn_find_file_in_prefix(prefix, name);

    strfreev(prefix);
    return path;
}

static char *srn_try_to_find_config_file(const char *name){
    char *path;
    char *prefix[] = {
        g_build_filename(PACKAGE_CONFIG_DIR, PACKAGE, NULL),
        g_build_filename(srn_get_executable_dir(), "etc", PACKAGE, NULL),
        g_build_filename(srn_get_executable_dir(), "..", "etc", PACKAGE, NULL),
        srn_get_executable_dir(),
        NULL
    };

    path = srn_find_file_in_prefix(prefix, name);

    strfreev(prefix);
    return path;
}

static char *srn_try_to_find_user_file(const char *name){
    char *path;
    char *prefix[] = {
        g_build_filename(g_get_user_config_dir(), PACKAGE, NULL),
        srn_get_executable_dir(),
        NULL
    };

    path = srn_find_file_in_prefix(prefix, name);

    strfreev(prefix);
    return path;
}

/**
 * @brief srn_get_theme_file returns the path of theme file with specified name.
 *  If the file is not found, returns NULL.
 *
 * @param fname
 *
 * @return NULL or path to the theme file, must be freed by g_free.
 */
char *srn_get_theme_file(const char *name){
    char *suffix;
    char *path;

    suffix = g_build_filename("themes", name, NULL);
    if (!suffix){
        return NULL;
    }

    path = srn_try_to_find_data_file(suffix);

    g_free(suffix);
    return path;
}

char *srn_get_user_config_file(){
    char *path;
    SrnRet ret;

    path = srn_try_to_find_data_file("srain.cfg");
    if (!path){
        path = g_build_filename(g_get_user_config_dir(), PACKAGE, "srain.cfg", NULL);
        if (!path) {
            return NULL;
        }

        ret = create_file_if_not_exist(path);
        if (!RET_IS_OK(ret)){
            WARN_FR("Failed to create user configuration file: %1$s", RET_MSG(ret));

            g_free(path);
            return NULL;
        }
    }

    return path;
}

char *srn_get_system_config_file(){
    char *path;

    path = srn_try_to_find_config_file("builtin.cfg");
    if (!path){
        // System configuration file should always exist
        WARN_FR("'%s' not found", path);
    }

    return path;
}

// FIXME: actually it only create the dir.
char *srn_create_log_file(const char *srv_name, const char *fname){
    char *path;
    char *tmp;
    SrnRet ret;

    tmp = srn_try_to_find_user_file("logs");
    if (tmp){
        // Directory 'logs' exists
        path = g_build_filename(tmp, srv_name, fname, NULL);
        g_free(tmp);
        tmp = NULL;
    } else {
        // $XDG_DATA_HOME/srain/logs/<srv_name>/<fname>
        path = g_build_filename(g_get_user_data_dir(), PACKAGE, "logs",
                                srv_name, fname, NULL);
    }

    if (!path){
        return NULL;
    }

    ret = create_file_if_not_exist(path);
    if (!RET_IS_OK(ret)){
        WARN_FR("Failed to create log file: %1$s", RET_MSG(ret));

        g_free(path);
        return NULL;
    }

    return path;
}

/**
 * @brief srn_create_user_files creates users files which required for
 *  running of Srain
 *
 * @return SRN_OK if all required files are created or already existent
 *
 * User fils including:
 *
 *  - $XDG_CONFIG_HOME/srain/
 *  - $XDG_CONFIG_HOME/srain/srain.cfg
 *  - $XDG_CACHE_HOME/srain/
 *  - $XDG_DATA_HOME/srain/logs
 *
 *  FIXME: not in use
 */
SrnRet srn_create_user_files(){
    SrnRet ret;
    char *path;

    path = g_build_filename( // $XDG_CONFIG_HOME/srain/
            g_get_user_config_dir(),
            PACKAGE,
            NULL);
    ret = create_dir_if_not_exist(path);
    g_free(path);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to create user configuration directory: %1$s"),
                RET_MSG(ret));
        goto FIN;
    }

    // $XDG_CONFIG_HOME/srain/srain.cfg
    path = g_build_filename(g_get_user_config_dir(), PACKAGE, "srain.cfg", NULL);
    ret = create_file_if_not_exist(path);
    g_free(path);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to create user configuration file: %1$s"),
                RET_MSG(ret));
        goto FIN;
    }

    // $XDG_CACHE_HOME/srain/
    path = g_build_filename(g_get_user_cache_dir(), PACKAGE, NULL);
    ret = create_dir_if_not_exist(path);
    g_free(path);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to create user cache directory: %1$s"),
                RET_MSG(ret));
        goto FIN;
    }

    // $XDG_DATA_HOME/srain/
    path = g_build_filename(g_get_user_data_dir(), PACKAGE, NULL);
    ret = create_dir_if_not_exist(path);
    g_free(path);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to create user data directory: %1$s"),
                RET_MSG(ret));
        goto FIN;
    }

    // $XDG_DATA_HOME/srain/logs
    path = g_build_filename(g_get_user_data_dir(), PACKAGE, "logs", NULL);
    ret = create_dir_if_not_exist(path);
    g_free(path);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to create chat logs directory: %1$s"),
                RET_MSG(ret));
        goto FIN;
    }

    ret = SRN_OK;
FIN:
    return ret;
}

SrnRet create_dir_if_not_exist(const char *path) {
    if (g_file_test(path, G_FILE_TEST_IS_DIR)){
        return SRN_OK;
    }
    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return RET_ERR(_("Not a directory"));
    }

    if (g_mkdir_with_parents(path, DEFAULT_DIR_MODE) == -1) {
        return RET_ERR("%s", g_strerror(errno));
    }

    return SRN_OK;
}

SrnRet create_file_if_not_exist(const char *path) {
    int fd;
    char *dir;
    SrnRet ret;

    if (g_file_test(path, G_FILE_TEST_IS_REGULAR)){
        return SRN_OK;
    }
    if (g_file_test(path, G_FILE_TEST_EXISTS)){
        return RET_ERR(_("Not a regular file"));
    }

    dir = g_path_get_dirname(path);
    ret = create_dir_if_not_exist(dir);
    g_free(dir);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    if ((fd = g_creat(path, DEFAULT_FILE_MODE)) == -1) {
        return RET_ERR("%s", g_strerror(errno));
    }
    g_close(fd, NULL);

    return SRN_OK;
}

#ifdef _WIN32
char *srn_get_executable_path() {
   char rawPathName[MAX_PATH];
   GetModuleFileNameA(NULL, rawPathName, MAX_PATH);
   return g_build_filename(rawPathName, NULL);
}

char *srn_get_executable_dir() {
    char *exePath = srn_get_executable_path();
    char *executableDir = g_path_get_dirname(exePath);
    g_free(exePath);
    return executableDir;
}

#elif defined __linux__
char *srn_get_executable_path() {
   char rawPathName[PATH_MAX];
   realpath(PROC_SELF_EXE, rawPathName);
   return g_build_filename(rawPathName, NULL);
}

char *srn_get_executable_dir() {
    char *exePath = srn_get_executable_path();
    char *executableDir = g_path_get_dirname(exePath);
    g_free(exePath);
    return executableDir;
}

#elif defined __APPLE__
char *srn_get_executable_path() {
    char rawPathName[PATH_MAX];
    char realPathName[PATH_MAX];
    uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);

    if(!_NSGetExecutablePath(rawPathName, &rawPathSize)) {
        realpath(rawPathName, realPathName);
    }
    return g_build_filename(realPathName, NULL);
}

char *srn_get_executable_dir() {
    char *executablePath = srn_get_executable_path();
    char *executableDir = g_path_get_dirname(executablePathStr);
    g_free(executablePath);
    return executableDir;
}

#elif defined __FreeBSD__
char *srn_get_executable_path() {
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
    char rawPath[PATH_MAX];
    char realPath[PATH_MAX];
    size_t sz = sizeof(rawPath);

    if(sysctl(mib, 4, rawPath, &sz, NULL, 0) == -1) {
        return NULL;
    }
    realpath(rawPath, realPath);
    return g_strdup(realPath);
}

char *srn_get_executable_dir() {
    char *executablePath = srn_get_executable_path();
    char *executableDir = g_path_get_dirname(executablePath);
    g_free(executablePath);
    return executableDir;
}
#endif
