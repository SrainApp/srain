/**
 * @file markup.c
 * @brief 
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-16
 */

#define __LOG_ON

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include "log.h"

#define MAX_ERROR_MSG 0x1000

static int compile_regex(regex_t *r, const char *pattern)
{
    int stat;
	char err_msg[MAX_ERROR_MSG];

    stat = regcomp(r, pattern, REG_EXTENDED|REG_NEWLINE);
    if (stat!= 0) {
        regerror(stat, r, err_msg, MAX_ERROR_MSG);
        ERR_FR("'%s': %s\n", pattern, err_msg);
        return 1;
    }

    return 0;
}

void markup(const char *msg){
    int n = 10;
    int stat, start, end;
    char *buf_ptr;
    const char *msg_ptr;
    char buf[512];
    char url[200];
    char url_buf[512];
    char pattern[] = "((http)|(https))://(www)?.[.-_[:alnum:]]+";   // TODO
    regex_t re;
    regmatch_t match[n];

    compile_regex(&re, pattern);

    msg_ptr = msg;
    buf_ptr = buf;
    memset(buf, 0, sizeof(buf));
    while (1){
        int no_match = regexec(&re, msg_ptr, n, match, 0);

        if (no_match){
            LOG_FR("no more matched");
            break;
        }

        if (match[0].rm_so == -1){
            break;
        }

        start = match[0].rm_so + (msg_ptr - msg);
        end = match[0].rm_eo + (msg_ptr - msg);

        strncpy(buf_ptr, msg_ptr, msg + start - msg_ptr);

        memset(url, 0, sizeof(url));
        strncpy(url, msg + start, end - start);

        memset(url_buf, 0, sizeof(url_buf));
        snprintf(url_buf, 512, "<a href=\"%s\">%s</a>", url, url);

        strcat(buf_ptr, url_buf);

        LOG_FR("'%.*s' (bytes %d:%d)", (end - start), msg + start, start, end);

        buf_ptr += strlen(buf_ptr);
        if (buf_ptr - buf > sizeof(buf)){
            ERR_FR("buffer overflow");
            return;
        }
        msg_ptr += match[0].rm_eo;
    }

    LOG_FR("BUF: %s", buf);
}
