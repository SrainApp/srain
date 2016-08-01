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

#include "srv.h"
#include "srv_test.h"

#include "srain_app.h"

#include "i18n.h"
#include "log.h"

/**
 * @brief Create directories and config files if no exist
 *          - $XDG_CONFIG_HOME/srain/
 *          - $XDG_CONFIG_HOME/srain/srainrc
 *          - $XDG_CACHE_HOME/srain/
 *
 * @return 0 if all required files are created or already existent
 */
static int create_user_file(){
    int res;
    FILE *fp;
    char *congif_dir;
    char *cache_dir;
    char *rc_file;

    congif_dir = g_build_filename(g_get_user_config_dir(), "srain", NULL);
    res = mkdir(congif_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("failed to create directory '%s', errno %d", congif_dir, errno);
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
            ERR_FR("failed to create file '%s', errno %d", rc_file, errno);
            exit(errno);
        }
    }
    g_free(rc_file);
    rc_file = NULL;

    cache_dir = g_build_filename(g_get_user_cache_dir(), "srain", NULL);
    res = mkdir(cache_dir, 0700);
    if (res == -1) {
        if (errno != EEXIST){
            ERR_FR("failed to create directory '%s', errno %d", cache_dir, errno);
            return errno;
        }
    }
    g_free(cache_dir);
    cache_dir = NULL;

    return 0;
}

int main(int argc, char **argv){
    create_user_file();
    i18n_init();

#ifndef UI_TEST
    srv_init();
#endif

#ifndef IRC_TEST
    ui_init(argc, argv);
#endif
}
