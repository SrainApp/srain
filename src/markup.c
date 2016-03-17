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
#include <glib.h>
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

GString* markup(const char *msg){
    int n = 10;
    int start, end;
    const char *msg_ptr;
    /* if `-` is contained in pattern,
     * regcomp() return ERROR "不适用的范围结束"  TODO*/
    char pattern[] = "((http)|(https))://(www)?[./;?:@&=+$,_!~*'[:alnum:]]+";
    regex_t re;
    regmatch_t match[n];
    GString *str;

    if (compile_regex(&re, pattern) == -1){
        return NULL;
    }

    msg_ptr = msg;
    str = g_string_new(NULL);
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

        str = g_string_append_len(str, msg_ptr, msg + start - msg_ptr);
        g_string_append_printf(str, "<a href=\"%.*s\">%.*s</a>",
                end - start, msg + start, end -start, msg + start);

        LOG_FR("match '%.*s' (%d:%d)", (end - start), msg + start, start, end);

        msg_ptr += match[0].rm_eo;
    }

    str = g_string_append(str, msg_ptr);

    return str;
}
