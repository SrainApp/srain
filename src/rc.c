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
#include "utils.h"

SrnRet rc_read(){
    int nline = 1;
    char *path = NULL;
    char *line = NULL;
    gsize len = 0;
    GError *err = NULL;
    GFile *file = NULL;
    GFileInputStream *fins = NULL;
    GDataInputStream *dins = NULL;
    SrnRet ret = SRN_OK;

    path = get_config_file("srainrc");
    if (!path) {
        ret = RET_ERR(_("Rc file not found"));
        goto fin;
    }

    file = g_file_new_for_path(path);

    err = NULL;
    fins = g_file_read(file, NULL, &err);
    if (err) {
        ret = RET_ERR(_("%s"), err->message);
        goto fin;
    }

    dins = g_data_input_stream_new(G_INPUT_STREAM(fins));

    for (;;) {
        err = NULL;
        line = g_data_input_stream_read_line(dins, &len, NULL, &err);

        if (err) {
            ret = RET_ERR(_("%s"), err->message);
            goto fin;
        }
        if (!line) {
            break;
        }
        if (line[0] == '#' || str_is_empty(line)) {
            g_free(line);
            continue;
        }

        ret = server_cmd(NULL, line);
        if (!RET_IS_OK(ret)){
            ret = RET_ERR(_("Command failed at line %d: %s"), nline, RET_MSG(ret));
            g_free(line);
            goto fin;
        }
        nline++;
    }

fin:
    if (path) {
        g_free(path);
    }
    if (file){
        g_object_unref(file);
    }
    if (fins) {
        g_object_unref(fins);
    }
    if (dins) {
        g_object_unref(dins);
    }
    return ret;
}
