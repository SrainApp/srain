/**
 * @file rc.c
 * @brief run command file reader
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * read $XDG_CONFIG_HOME/srain/srainrc or $PWD/srainrc,
 * call `srain_cmd` for every line
 */

#define __LOG_ON

#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include "log.h"
#include "srain.h"
#include "meta.h"

int rc_read(){
    FILE *fp;
    char *line;
    size_t len;
    ssize_t read;
    char *rc_path;


    /* try to open $PWD/srainrc */
    rc_path = g_build_filename(g_get_current_dir(), "srainrc", NULL);
    LOG_F("opening '%s'... ", rc_path);
    fp = fopen(rc_path, "r");
    g_free(rc_path);

    if (fp) goto ready;
    LOG("failed\n")

    /* try to open $PWD/srainrc */
    rc_path = g_build_filename(g_get_user_config_dir(), "srainrc", NULL);
    LOG_F("opening '%s'... ", rc_path);
    fp = fopen(rc_path, "r");
    g_free(rc_path);

    if (fp) goto ready;
    LOG("failed, new one\n")

    rc_path = g_build_filename(g_get_user_config_dir(), "srainrc", NULL);
    LOG_F("creating '%s'... ", rc_path);
    fp = fopen(rc_path, "w");
    g_free(rc_path);
    if (!fp) {
        ERR("failed, exit :(\n");
        exit(-1);
    }

ready:
    LOG("done\n");
    len = 0;
    line = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line){
            strtok(line, "\n");
            LOG_FR("read: %s", line);
            srain_cmd(META_SERVER, line);
        }
    }

    if (line) free(line);
    fclose(fp);

    return 0;
}
