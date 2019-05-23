/* Copyright (C) 2016-2017 Z.Wind.L <zwindl@protonmail.com>
 * Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/core.h"
#include "markup_renderer.h"
#include "pattern.h"

#include "./filter.h"

#define PATTERNS_KEY "pattern_filter_patterns"

static void init(void);
static void finalize(void);
static bool filter(const SrnMessage *msg);
static GList** alloc_patterns();
static void free_patterns(GList **patterns);
static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);

static SrnMarkupRenderer *markup_renderer;

/**
 * @brief pattern_filter is a filter module for filtering message which matches
 * given pattern.
 */
SrnMessageFilter pattern_filter = {
    .name = "pattern",
    .init = init,
    .finalize = finalize,
    .filter = filter,
};

void init(void) {
    GMarkupParser *parser;

    markup_renderer = srn_markup_renderer_new();
    parser = srn_markup_renderer_get_markup_parser(markup_renderer);
    parser->start_element = NULL;
    parser->end_element = NULL;
    parser->text = text;
}

void finalize(void) {
    srn_markup_renderer_free(markup_renderer);
}

static bool filter(const SrnMessage *msg) {
    char *raw_content;
    GList *patterns;
    GList **patterns1;
    GList **patterns2;
    GList *lst;
    SrnRet ret;

    g_return_val_if_fail(msg->chat && msg->chat->srv, SRN_ERR);

    patterns = NULL;
    patterns1 = srn_chat_get_extra_data(msg->chat, PATTERNS_KEY);
    patterns2 = srn_chat_get_extra_data(msg->chat->srv->chat, PATTERNS_KEY);

    if (patterns1 && *patterns1) {
        patterns = g_list_concat(patterns, g_list_copy(*patterns1));
    }
    if (patterns2 && *patterns2) {
        patterns = g_list_concat(patterns, g_list_copy(*patterns2));
    }

    raw_content = NULL;
    ret = srn_markup_renderer_render(markup_renderer,
            msg->rendered_content, &raw_content, NULL);
    if (!RET_IS_OK(ret)){
        // ret = RET_ERR(_("Failed to render markup text: %1$s"), RET_MSG(ret));
        // return TRUE;
    }
    lst = patterns;
    while (lst) {
        const char *pattern;
        const GRegex *regex;

        pattern = lst->data;
        regex = srn_pattern_get_regex(pattern);
        if (regex) {
             if (g_regex_match(regex, raw_content, 0, NULL)) {
                 return FALSE;
             }
        }
        lst = g_list_next(lst);
    }

    g_list_free(patterns);

    // TODO
    return TRUE;
}

SrnRet srn_pattern_filter_add_pattern(SrnChat *chat, const char *pattern){
    GList **patterns;
    GList *lst;

    patterns = srn_chat_get_extra_data(chat, PATTERNS_KEY);
    if (!patterns) {
        patterns = alloc_patterns();
        srn_chat_set_extra_data(chat, PATTERNS_KEY, patterns,
                (GDestroyNotify)free_patterns);
    }

    lst = *patterns;
    while (lst) {
        if (g_strcasecmp(lst->data, pattern) == 0) {
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    *patterns = g_list_append(*patterns, g_strdup(pattern));

    return SRN_OK;
}

SrnRet srn_pattern_filter_rm_pattern(SrnChat *chat, const char *pattern){
    GList **patterns;
    GList *lst;

    patterns = srn_chat_get_extra_data(chat, PATTERNS_KEY);
    g_return_val_if_fail(patterns, SRN_OK);

    lst = *patterns;
    while (lst) {
        if (g_strcasecmp(lst->data, pattern) == 0) {
            break;
        }
        lst = g_list_next(lst);
    }
    if (!lst) {
        return SRN_ERR;
    }

    g_free(lst->data);
    *patterns = g_list_delete_link(*patterns, lst);

    return SRN_OK;
}

static GList** alloc_patterns()  {
    return g_malloc0(sizeof(GList **));
}

static void free_patterns(GList **patterns){
    if (patterns) {
        if (*patterns) {
            g_list_free_full(*patterns, g_free);
        }
        g_free(patterns);
    }
}

static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error){
    g_string_append_len(srn_markup_renderer_get_markup(user_data), text, text_len);
}
