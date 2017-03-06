/**
 * @file rc.c
 * @brief Run command from file
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * Read $DESTDIR/srain/srainrc or $XDG_CONFIG_HOME/srain/srainrc,
 * call `server_cmd()` for every line
 */

#define __LOG_ON

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "server_cmd.h"

#include "srain.h"
#include "meta.h"
#include "i18n.h"
#include "log.h"
#include "file_helper.h"

int rc_read(){
    FILE *fp;
    char *line;
    size_t len;
    ssize_t read;
    char *rc_file;

    rc_file = get_config_file("srainrc");
    if (!rc_file) return SRN_ERR;

    fp = fopen(rc_file, "r");

    if (!fp){
        ERR_FR("Failed to open %s", rc_file);
        g_free(rc_file);
        return;
    }
    g_free(rc_file);

    len = 0;
    line = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line && line[0] != '#'){
            strtok(line, "\n");
            if (server_cmd(NULL, NULL, line) != SRN_OK){
                ERR_FR("Command failed: %s", line);
                break;
            }
        }
    }

    if (line) free(line);

    fclose(fp);
}
