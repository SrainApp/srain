/**
 * @file chat_log.c
 * @brief Save chat log to local store
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-08-21
 */

// #define __DBG_ON
#define __LOG_ON

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <glib.h>

#include "meta.h"
#include "get_path.h"
#include "log.h"

// TODO: remove me
static void get_cur_time(const char *fmt, char *timestr){
    time_t curtime;

    time(&curtime);

    strftime(timestr, 32, fmt, localtime(&curtime));
    timestr[31] = '\0';
}

FILE *get_log_file(const char *srv_name, const char *filename){
    int res;
    char *path;
    FILE *fp;

    path = g_build_filename(g_get_user_data_dir(),
            PACKAGE, "logs", srv_name, filename, NULL);

    if (!g_file_test(path, G_FILE_TEST_EXISTS)){
        char *dir;

        /* Create if dir non-exist */
        dir = g_build_filename(g_get_user_data_dir(),
                PACKAGE, "logs", srv_name, NULL);
        res = mkdir(dir, 0700);
        if (res == -1) {
            if (errno != EEXIST){
                ERR_FR("Failed to create directory '%s', errno %d", dir, errno);
                g_free(dir);
                return NULL;
            }
        }
        g_free(dir);
    }

    fp = fopen(path, "a+");
    g_free(path);

    return fp;
}

void chat_log_log(const char *srv_name, const char *chan_name, const char *msg){
    FILE *fp;
    char timestr[32];
    char datestr[32];
    GString *basename;

    DBG_FR("srv_name: %s, chan_name: %s, msg: %s", srv_name, chan_name, msg);

    get_cur_time("%T", timestr);
    get_cur_time("%F", datestr);

    basename = g_string_new("");
    g_string_append_printf(basename, "%s.%s.log", datestr, chan_name);

    fp = get_log_file(srv_name, basename->str);
    fprintf(fp,"[%s] %s\n", timestr, msg);

    fclose(fp);
    g_string_free(basename, TRUE);
}

void chat_log_fmt(const char *srv_name, const char *chan_name, const char *fmt, ...){
    char msg[512];
    va_list args;

    if (strlen(fmt) != 0){
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);
    }

    chat_log_log(srv_name, chan_name, msg);
}
