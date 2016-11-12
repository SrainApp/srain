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
#include <sys/stat.h>

#include "ui.h"
#include "ui_hdr.h"

#include "meta.h"
#include "i18n.h"
#include "log.h"
#include "file_helper.h"

void _rc_read(){
    FILE *fp;
    char *line;
    size_t len;
    ssize_t read;
    char *rc_file;

    rc_file = get_config_file("srainrc");
    if (!rc_file) return;

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
        if (line){
            strtok(line, "\n");
            LOG_FR("Read: '%s'", line);
            if (ui_hdr_srv_cmd(NULL, line, 1) < 0){
                ERR_FR("Command failed: %s", line);

                char msg[512];
                snprintf(msg, sizeof(msg), _("Command \"%s\" failed"), line);
                ui_sys_msg(_("Null"), "", msg, SYS_MSG_ERROR, 0);
                break;
            }
        }
    }

    if (line) free(line);
    fclose(fp);
}

int rc_read(){
    char *rc_file;
    struct stat st;

    rc_file = get_config_file("srainrc");
    if (!rc_file) return -1;

    if (stat(rc_file, &st) != 0) {
        g_free(rc_file);
        return -1;
    }

    g_free(rc_file);

    if (st.st_size == 0) return -1;

    g_thread_new(NULL, (GThreadFunc)_rc_read, NULL);

    return 0;
}
