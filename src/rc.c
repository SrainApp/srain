/**
 * @file rc.c
 * @brief run command file reader
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * read $DESTDIR/srain/srainrc or $XDG_CONFIG_HOME/srain/srainrc,
 * call `srain_cmd` for every line
 */

#define __LOG_ON

#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include "ui_intf.h"
#include "log.h"
#include "meta.h"

int rc_read(){
    FILE *fp;
    char *line;
    size_t len;
    ssize_t read;
    char *rc_file;

    /* try opening  $PWD/srainrc , used when debug */
    LOG_F("try opening './srainrc'... ");
    fp = fopen("./srainrc", "r");

    if (!fp){
        LOG("failed\n");

        /* open $XDG_CONFIG_HOME/srainrc, it should exist
         * (created in src/main.c::main() )*/
        rc_file = g_build_filename(g_get_user_config_dir(), "srain", "srainrc", NULL);

        LOG_F("try opening '%s'... ", rc_file);
        fp = fopen(rc_file, "r");
        g_free(rc_file);
        if (!fp){
            LOG("failed\n");
            g_free(rc_file);
            return -1;
        }
    }
    LOG("ok\n");

    len = 0;
    line = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line){
            strtok(line, "\n");
            LOG_FR("read: '%s'", line);
            if (ui_intf_server_cmd(NULL, line) < 0){
                break;
            }
        }
    }

    if (line) free(line);
    fclose(fp);

    return 0;
}
