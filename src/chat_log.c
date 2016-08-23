/**
 * @file chat_log.c
 * @brief Save chat log to local store
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-08-21
 */

// #define __DBG_ON
#define __LOG_ON

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "meta.h"
#include "file_helper.h"
#include "log.h"

// TODO: remove me
static void get_cur_time(const char *fmt, char *timestr){
    time_t curtime;

    time(&curtime);

    strftime(timestr, 32, fmt, localtime(&curtime));
    timestr[31] = '\0';
}

void chat_log_log(const char *srv_name, const char *chan_name, const char *msg){
    FILE *fp;
    char *file;
    char timestr[32];
    char datestr[32];
    GString *basename;

    DBG_FR("srv_name: %s, chan_name: %s, msg: %s", srv_name, chan_name, msg);

    get_cur_time("%T", timestr);
    get_cur_time("%F", datestr);

    basename = g_string_new("");
    g_string_append_printf(basename, "%s.%s.log", datestr, chan_name);

    file = create_log_file(srv_name, basename->str);
    if (!file){
        ERR_FR("Failed to create log file");
        goto cleanup1;
    }

    fp = fopen(file, "a+");
    if (!fp){
        ERR_FR("Failed to open file '%s'", file)
        goto cleanup2;
    }
    fprintf(fp,"[%s] %s\n", timestr, msg);

    fclose(fp);
cleanup2:
    g_free(file);
cleanup1:
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
