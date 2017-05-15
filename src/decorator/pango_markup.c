/**
 * @file pango_markup.c
 * @brief Decorator for escaping message and render url
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-03-11
 */

// #define __DBG_ON
// #define __LOG_ON

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "decorator.h"

#include "log.h"

/* Some patterns are copied from hexchat/src/common/url.c */
#define PROTO_PATTERN       "(http|https|ftp|git|svn|irc|xmpp)"

#define DOMAIN_PATTERN      "[_\\pL\\pN\\pS][-_\\pL\\pN\\pS]*(\\.[-_\\pL\\pN\\pS]+)*"

/* Ref: https://w3techs.com/technologies/overview/top_level_domain/all */
#define TLD_PATTERN         "\\.(com|ru|org|net|de|jp|uk|br|it|pl|fr|in|au|ir"  \
                            "|info|nl|cn|es|cz|kr|ca|eu|ua|co|gr|ro|za|biz|ch"  \
                            "|se|tw|mx|vn|hu|be|at|tr|dk|tv|me|ar|sk|no|us|fi"  \
                            "|id|cl|xyz|io|pt|by|il|ie|nz|kz|hk|lt|cc|my|sg"    \
                            "|club|bg|рф|edu|top|pk|su|th|hr|rs|pro|pe|si|az"   \
                            "|lv|pw|ae|ph|ng|online|ee|ve|cat|moe|tk|ml)"
#define IP_PATTERN          "[0-9]{1,3}(\\.[0-9]{1,3}){3}"

#define PORT_PATTERN        "(:[1-9][0-9]{0,4})"
#define HOST_PATTERN        "(" DOMAIN_PATTERN TLD_PATTERN "|" IP_PATTERN ")" PORT_PATTERN "?"

/* For convenience, last character of URL is limited */
#define URL_PATH_PATTERN    "(/[A-Za-z0-9-_.~:/?#\\[\\]@!&'()*+,;=%|]*[A-Za-z0-9-_/])?/?"
#define URL_PATTERN         PROTO_PATTERN "://" HOST_PATTERN URL_PATH_PATTERN

/* Ref: https://tools.ietf.org/html/rfc1459#section-1.3
   For convenience, last character of channel is limited */
#define CHANNEL_PATTERN     "[#&][^\x07\x2C\\s,:]{0,199}[A-Za-z0-9-_+]"

#define EMAIL_PATTERN       "[a-z0-9][._+%a-z0-9-]+@" HOST_PATTERN

static int pango_markup(Message *msg, DecoratorFlag flag, void *user_data);

Decorator pango_markup_decroator = {
    .name = "pango_markup",
    .func = pango_markup,
};

typedef enum {
    MATCH_URL,
    MATCH_HOST,
    MATCH_CHANNEL,
    MATCH_EMAIL,

    /* ... */
    MATCH_MAX,
} MatchType;

static char* patterns[MATCH_MAX + 1] = {
    [MATCH_URL] = URL_PATTERN,
    [MATCH_CHANNEL] = CHANNEL_PATTERN,
    [MATCH_EMAIL] = EMAIL_PATTERN,
    [MATCH_HOST] = HOST_PATTERN,
};

static bool match_pattern(const char *pattern, const char *str, int *start, int *end) {
    bool ret;
    GError *err;
    GRegex *regex;
    GMatchInfo *match_info;

    err = NULL;
    regex = g_regex_new(pattern, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed, pattern: %s, err: %s", pattern, err->message);
        return FALSE;
    }

    g_regex_match(regex, str, 0, &match_info);

    if (!(ret = g_match_info_matches(match_info))){
        goto fin;
    }

    ret = g_match_info_fetch_pos(match_info, 0, start, end);

fin:

    g_match_info_free(match_info);
    g_regex_unref(regex);

    return ret;
}

static int pango_markup(Message *msg, DecoratorFlag flag, void *user_data){
    int start, end;
    int tmpstart, tmpend;
    char *left;
    char *ptr, *ptrend;
    char *url, *markuped_url;
    GString *dcontent;
    MatchType type;

    ptr = msg->dcontent;
    ptrend = msg->dcontent + strlen(msg->dcontent);
    dcontent = g_string_new(NULL);

    while (ptr < ptrend) {
        type = MATCH_MAX;
        start = end = strlen(ptr);

        for (int i = 0; i < MATCH_MAX; i++){
            if (!patterns[i]) continue;

            if (match_pattern(patterns[i], ptr, &tmpstart, &tmpend)){
                DBG_FR("Temp Match[%d,%d): %.*s ",
                        tmpstart, tmpend, tmpend - tmpstart, ptr + tmpstart);

                if (tmpstart < start){
                    start = tmpstart;
                    end = tmpend;
                    type = i;
                }
            }
        }

        /* Markup the left of the matched(maybe) url */
        left = g_markup_escape_text(ptr, start);
        dcontent = g_string_append(dcontent, left);
        g_free(left);

        /* If something matched */
        if (type != MATCH_MAX){
            url = g_strndup(ptr + start, end - start);

            DBG_FR("Match url: %s, type: %d", url, type);

            switch(type){
                case MATCH_URL:
                case MATCH_HOST:
                    markuped_url = g_markup_printf_escaped("<a href=\"%s\">%s</a>", url, url);
                    break;
                case MATCH_CHANNEL:
                    markuped_url = g_markup_printf_escaped("<a href=\"irc://%s:%d/%s\">%s</a>",
                            msg->chat->srv->info->host,
                            msg->chat->srv->info->port,
                            url + 1,
                            url);
                    break;
                case MATCH_EMAIL:
                    markuped_url = g_markup_printf_escaped("<a href=\"mailto:%s\">%s</a>", url, url);
                    break;
                default:
                    markuped_url = NULL;
                    break;
            }

            msg->urls = g_slist_append(msg->urls, url);
            dcontent = g_string_append(dcontent, markuped_url);

            DBG_FR("Appended url: %s", url);
            DBG_FR("Markuped url: %s", markuped_url);

            g_free(markuped_url);
        }

        ptr += end;
    }

    LOG_FR("Result: %s", dcontent->str);

    g_free(msg->dcontent);
    msg->dcontent = dcontent->str;
    g_string_free(dcontent, FALSE);

    return SRN_OK;
}
