/**
 * @file rc.c
 * @brief run command file reader
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * read $DESTDIR/srain/srainrc or $XDG_CONFIG_HOME/srain/srainrc,
 * call `ui_hdr_srv_cmd()` for every line
 */

#define __LOG_ON

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "srv.h"

#include "meta.h"
#include "log.h"
#include "get_path.h"

int rc_read(){
    FILE *fp;
    char *line;
    size_t len;
    ssize_t read;
    char *rc_file;


    rc_file = get_config_path("srainrc");

    if (!rc_file){
        return -1;
    }

    fp = fopen(rc_file, "r");

    if (!fp){
        ERR_FR("Failed to open %s", rc_file);
        g_free(rc_file);

        return -1;
    }
    g_free(rc_file);

    len = 0;
    line = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line){
            strtok(line, "\n");
            LOG_FR("Read: '%s'", line);
            if (srv_cmd(NULL, NULL, line) < 0){
                ERR_FR("Command failed");
                break;
            }
        }
    }

    if (line) free(line);
    fclose(fp);

    return 0;
}
