/**
 * @file main.c
 * @brief main function here
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <sys/stat.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "srv.h"
#include "srv_test.h"

#include "srain_app.h"

#include "i18n.h"
#include "log.h"
#include "plugin.h"

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
static int create_user_file(){
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

void quit(){
    plugin_finalize();

#ifdef IRC_TEST
    srv_finalize();
#endif

    exit(0);
}

int main(int argc, char **argv){
    create_user_file();
    signal(SIGINT, quit);

    i18n_init();
    plugin_init();

#ifndef UI_TEST
    srv_init();
#endif

#ifndef IRC_TEST
    ui_init(argc, argv);
#endif

    quit();

    return 0;
}
