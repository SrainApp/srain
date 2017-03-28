/**
 * @file regex.c
 * @brief Regex message filter
 * @author Z.Wind.L <zwindl@protonmail.com>
 *         Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-03-16
 */

// #define __DBG_ON
// #define __LOG_ON

#include <glib.h>
#include <strings.h>

#include "server.h"

#include "srain.h"
#include "filter.h"
#include "i18n.h"
#include "log.h"

typedef struct _NamedPattern {
    char *name;
    char *pattern;
} NamedPattern;

static void named_pattern_free(NamedPattern *np);
static bool regex(const Message *msg, FilterFlag flag, void *user_data);

Filter regex_filter = {
    .name = "regex",
    .func = regex,
};

int regex_filter_add_pattern(Chat *chat, const char *name, const char *pattern){
    GSList *lst;
    GError *err;
    GRegex *regex;
    NamedPattern *np;

    lst = chat->ignore_regex_list;

    while(lst){
        if (lst->data){
            np = lst->data;
            if (strcasecmp(np->name, name) == 0){
                chat_add_error_message_fmt(chat, NULL,
                        _("\"%s\" already exists in %s 's regex list"),
                        np->name, chat->name);
                return SRN_ERR;
            }
        }
        lst = g_slist_next(lst);
    }

    err = NULL;
    regex = g_regex_new(pattern, 0, 0, &err);
    if (!regex){
        chat_add_error_message_fmt(chat, NULL,
                _("Invail pattern: %s"), err->message);
        return SRN_ERR;
    }

    g_regex_unref(regex);

    np = g_malloc0(sizeof(NamedPattern));
    np->name = g_strdup(name);
    np->pattern = g_strdup(pattern);

    chat->ignore_regex_list = g_slist_append(chat->ignore_regex_list, np);

    chat_add_misc_message_fmt(chat, NULL,
            _("\"%s\" has added to %s 's regex list"), np->name, chat->name);

    return SRN_OK;
}

int regex_filter_rm_pattern(Chat *chat, const char *name){
    GSList *lst;
    NamedPattern *np;

    lst = chat->ignore_regex_list;

    while(lst){
        if (lst->data){
            np = lst->data;
            if (strcasecmp(np->name, name) == 0){
                chat_add_misc_message_fmt(chat, NULL,
                        _("\"%s\" is removed from %s 's ignore list"),
                        name, chat->name);

                named_pattern_free(np);
                chat->ignore_regex_list = g_slist_delete_link(chat->ignore_regex_list, lst);

                return SRN_OK;
            }
        }
        lst = g_slist_next(lst);
    }

    chat_add_error_message_fmt(chat, NULL,
            _("\"%s\" not found in %s 's ignore list"),
            name, chat->name);

    return SRN_ERR;
}

void regex_filter_free(Chat *chat, const char *regex){
    g_slist_free_full(chat->ignore_regex_list,
            (GDestroyNotify)named_pattern_free);
    chat->ignore_regex_list = NULL;
}

bool regex(const Message *msg, FilterFlag flag, void *user_data){
    int i;
    GSList *lst;
    NamedPattern *np;

    for (i = 0, lst = msg->chat->ignore_regex_list;
            i < 2;
            i++, lst = msg->chat->srv->chat->ignore_regex_list){
        while (lst){
            if (lst->data){
                np = lst->data;
                if(g_regex_match_simple(np->pattern, msg->content, 0, 0)){
                    return FALSE;
                }
            }
            lst = g_slist_next(lst);
        }
    }

    return TRUE;
}

void named_pattern_free(NamedPattern *np){
    g_free(np->name);
    g_free(np->pattern);
    g_free(np);
}
