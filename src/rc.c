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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "server_cmd.h"

#include "srain.h"
#include "meta.h"
#include "i18n.h"
#include "log.h"
#include "rc.h"
#include "file_helper.h"

SrnRet rc_read(){
    int nline;
    FILE *fp = NULL;
    char *line;
    size_t len;
    ssize_t read;
    char *rc_file = NULL;
    SrnRet ret = SRN_OK;

    rc_file = get_config_file("srainrc");
    if (!rc_file) {
        ret = ERR(_("Rc file not found: %s"), rc_file);
        goto fin;
    }

    fp = fopen(rc_file, "r");
    if (!fp){
        ret = ERR(_("Can not open rc file: %s"), rc_file);
        goto fin;
    }

    len = 0;
    nline = 1;
    line = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line && line[0] != '#'){
            strtok(line, "\n");
            if ((ret = server_cmd(NULL, line)) != SRN_OK){
                ret = ERR("Command failed at line %d: %s", nline, ERRMSG(ret));
                break;
            }
        }
        nline++;
    }

fin:
    if (rc_file) {
        g_free(rc_file);
    }
    if (fp){
        fclose(fp);
    }
    if (line) {
        free(line);
    }
    return ret;
}
