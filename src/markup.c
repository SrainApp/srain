/**
 * @file markup.c
 * @brief match url and add html tags in string
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-16
 */

#define __LOG_ON

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include "srain_magic.h"
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
        return -1;
    }

    return 0;
}

// TODO return a GString?
void markup(const char *msg, char *markuped_msg, int len){
    int n = 10;
    int stat, start, end;
    char *buf_ptr;
    const char *msg_ptr;
    char url[200];
    char url_buf[512];
    /* dead simple regex pattern to match url TODO */
    char pattern[] = "((http)|(https))://(www)?[[:punct:][:alnum:]]+";
    regex_t re;
    regmatch_t match[n];

    if (compile_regex(&re, pattern) == -1){
        return;
    }

    msg_ptr = msg;
    buf_ptr = markuped_msg;
    memset(markuped_msg, 0, len);
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

        if (buf_ptr + strlen(url_buf) - markuped_msg >= len){
            ERR_FR("buffer full");
            return;
        }

        strncat(buf_ptr, url_buf, markuped_msg + len - buf_ptr);
        buf_ptr += strlen(buf_ptr);

        LOG_FR("'%.*s' (bytes %d:%d)", (end - start), msg + start, start, end);

        msg_ptr += match[0].rm_eo;
    }

    strncpy(buf_ptr, msg_ptr, markuped_msg + len - buf_ptr);

    LOG_FR("markuped: '%s'", markuped_msg);
    return;
}
