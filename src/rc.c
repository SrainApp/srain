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
    int nline = 1;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *rc_file = NULL;
    SrnRet ret = SRN_OK;

    rc_file = get_config_file("srainrc");
    if (!rc_file) {
        ret = RET_ERR(_("User rc file not found"));
        goto fin;
    }

    fp = fopen(rc_file, "r");
    if (!fp){
        ret = RET_ERR(_("Can not open rc file: %s"), rc_file);
        goto fin;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        if (line && line[0] != '#'){
            strtok(line, "\n");
            if (!RET_IS_OK(ret = server_cmd(NULL, line))){
                ret = RET_ERR(_("Command failed at line %d: %s"), nline, RET_MSG(ret));
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
