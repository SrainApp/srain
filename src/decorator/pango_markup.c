/**
 * @file markup.c
 * @brief provide function markup(): match url and
 *      return a GString with html tags
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-16
 */

// #define __LOG_ON

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <glib.h>
#include "log.h"

#define MAX_ERROR_MSG 0x1000

/* 人生苦短， 我用 GString */

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

/* pango markup language escape */
static GString* escape(const char *str){
    int i;
    int len;
    GString *str2;

    len = strlen(str);
    str2 = g_string_new(NULL);

    for (i = 0; i < len; i++){
        switch (str[i]){
            case '<':
                str2 = g_string_append(str2, "&lt;");
                break;
            case '>':
                str2 = g_string_append(str2, "&gt;");
                break;
            case '&':
                str2 = g_string_append(str2, "&amp;");
                break;
            default:
                str2 = g_string_append_c(str2, str[i]);
        }
    }

    return str2;
}

/* match the urls in `msg`, return a GString with html tags,
 * when img_url is not NULL, return the first url which
 * points to a image file via `img_url`
 */
GString* markup(const char *raw_msg, GString **img_url){
    int n = 10;
    int start, end;
    const char *msg;
    const char *msg_ptr;
    regex_t re;
    regmatch_t match[n];
    GString *url;
    GString *str;
    GString *escaped_msg;
    /* TODO: if `-` and `&` contained in pattern,
     * TODO: regcomp() may return ERROR "不适用的范围结束" */
    char pattern[] = "((http)|(https))://(www)?[-./;?:@&=+$,_!~*'#%[:alnum:]]+";

    if (compile_regex(&re, pattern) == -1){
        return NULL;
    }

    escaped_msg = escape(raw_msg);
    msg = escaped_msg->str;

    msg_ptr = msg;
    str = g_string_new(NULL);

    if (img_url) *img_url = NULL;

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
        url = g_string_new_len(msg + start, end - start);
        g_string_append_printf(str,
                               "<a href=\"%s\">%s</a>",
                               url->str,
                               url->str);

        LOG_FR("match '%.*s' (%d:%d)", (end - start), msg + start, start, end);

        /* is first url point to a image? */
        if (img_url && *img_url == NULL &&
                (strcmp(url->str + url->len - 4, ".png") == 0
                || strcmp(url->str + url->len - 4, ".jpg") == 0
                || strcmp(url->str + url->len - 5, ".jpeg") == 0
                || strcmp(url->str + url->len - 4, ".jeg") == 0)){
            LOG_FR("img found: '%s'", url->str);
            *img_url = url;
        } else {
            g_string_free(url, TRUE);
        }

        msg_ptr += match[0].rm_eo;
    }

    str = g_string_append(str, msg_ptr);

    g_string_free(escaped_msg, TRUE);

    return str;
}
