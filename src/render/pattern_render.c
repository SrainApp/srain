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
#include "pattern_set.h"

#include "./renderer.h"

#define PATTERNS_KEY "pattern_render_module_patterns"

static void init(void);
static void finalize(void);
static SrnRet render(SrnMessage *msg);
static GList** alloc_patterns();
static void free_patterns(GList **patterns);
static GList* get_patterns(SrnMessage *msg);
static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);

static SrnMarkupRenderer *markup_renderer;

/**
 * @brief pattern_renderer is a render module for extracting text from message
 * content via given pattern, and use them as new message content.
 *
 * NOTE: Make sure pattern_renderer is executed as first renderer.
 */
SrnMessageRenderer pattern_renderer = {
    .name = "pattern",
    .init = init,
    .finalize = finalize,
    .render = render,
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

static SrnRet render(SrnMessage *msg) {
    char *raw_content;
    GList *patterns;
    GList *lst;
    SrnRet ret;
    SrnPatternSet *pattern_set;

    pattern_set = srn_application_get_default()->pattern_set;
    g_return_val_if_fail(pattern_set, SRN_ERR);

    patterns = get_patterns(msg);

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
        regex = srn_pattern_set_get(pattern_set, pattern);
        if (regex) {
            GMatchInfo *match_info;

            match_info = NULL;
            g_regex_match(regex, raw_content, 0, &match_info);
            if (g_match_info_matches(match_info)) {
                char *sender;
                char *content;
                char *time;

                sender = g_match_info_fetch_named(match_info, "sender");
                content = g_match_info_fetch_named(match_info, "content");
                time = g_match_info_fetch_named(match_info, "time");

                if (sender) {
                    g_free(msg->rendered_remark);
                    msg->rendered_remark = msg->rendered_sender;
                    msg->rendered_sender = g_markup_escape_text(sender, -1);
                }
                if (content) {
                    g_free(msg->rendered_content);
                    msg->rendered_content = g_markup_escape_text(content, -1);
                }
                if (time) {
                    g_free(msg->rendered_short_time);
                    msg->rendered_short_time = g_markup_escape_text(time, -1);
                }

                g_free(sender);
                g_free(content);
                g_free(time);
                g_match_info_free(match_info);
            }
        }
        lst = g_list_next(lst);
    }

    g_list_free(patterns);

    return SRN_OK;
}

/**
 * @brief srn_render_attach_pattern attach a pattern name to given SrnExtraData.
 * The attached pattern name will be used to render message.
 *
 * @param extra_data
 * @param pattern is name of pattern which can be found i
 * SrnApplication->pattern_set.
 *
 * @return
 */
SrnRet srn_render_attach_pattern(SrnExtraData *extra_data, const char *pattern){
    GList **patterns;
    GList *lst;

    patterns = srn_extra_data_get(extra_data, PATTERNS_KEY);
    if (!patterns) {
        patterns = alloc_patterns();
        srn_extra_data_set(extra_data, PATTERNS_KEY, patterns,
                (GDestroyNotify)free_patterns);
    }

    lst = *patterns;
    while (lst) {
        if (g_ascii_strcasecmp(lst->data, pattern) == 0) {
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    *patterns = g_list_append(*patterns, g_strdup(pattern));

    return SRN_OK;
}

SrnRet srn_render_detach_pattern(SrnExtraData *extra_data, const char *pattern){
    GList **patterns;
    GList *lst;

    patterns = srn_extra_data_get(extra_data, PATTERNS_KEY);
    g_return_val_if_fail(patterns, SRN_OK);

    lst = *patterns;
    while (lst) {
        if (g_ascii_strcasecmp(lst->data, pattern) == 0) {
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

/**
 * @brief get_patterns
 *
 * @return a list which contains const string, should be freed by g_list_free();
 */
static GList* get_patterns(SrnMessage *msg) {
    GList *iter;
    GList *datas; // List of SrnExtraData
    GList *patterns;

    datas = NULL;
    datas = g_list_append(datas, msg->sender->extra_data);
    datas = g_list_append(datas, msg->sender->srv_user->extra_data);
    datas = g_list_append(datas, msg->chat->extra_data);
    datas = g_list_append(datas, msg->chat->srv->chat->extra_data);

    patterns = NULL;
    iter = datas;
    while (iter) {
        SrnExtraData *data;
        GList **lp;

        data = iter->data;
        lp = srn_extra_data_get(data, PATTERNS_KEY);
        if (lp && *lp) {
            patterns = g_list_concat(patterns, g_list_copy(*lp));
        }
        iter = g_list_next(iter);
    }

    g_list_free(datas);

    return patterns;
}

static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error){
    g_string_append_len(srn_markup_renderer_get_markup(user_data), text, text_len);
}
