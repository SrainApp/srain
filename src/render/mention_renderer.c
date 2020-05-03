/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "core/core.h"
#include "markup_renderer.h"
#include "i18n.h"

#include "./renderer.h"

struct Context {
    SrnMessage *message;
    GRegex *pattern;
};

static void init(void);
static void finalize(void);
static SrnRet render(SrnMessage *msg);
static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);
static bool on_regex_replace(const GMatchInfo *match_info, GString *result,
        gpointer user_data);

static SrnMarkupRenderer *markup_renderer;

SrnMessageRenderer mention_renderer = {
    .name = "mention",
    .init = init,
    .finalize = finalize,
    .render = render,
};

void init(void) {
    GMarkupParser *parser;

    markup_renderer = srn_markup_renderer_new();
    parser = srn_markup_renderer_get_markup_parser(markup_renderer);
    parser->text = text;
}

void finalize(void) {
    srn_markup_renderer_free(markup_renderer);
}

SrnRet render(SrnMessage *msg) {
    char *nick = NULL;
    char *pattern = NULL;
    char *rendered_content = NULL;
    GError *err = NULL;
    GRegex *regex = NULL;
    SrnRet ret = SRN_OK;
    struct Context ctx;

    g_return_val_if_fail(msg->chat
            && msg->chat->srv
            && msg->chat->srv->user, SRN_ERR);

    if (msg->mentioned){
        return SRN_OK;
    }

    /* Genertate pattern */
    nick = g_regex_escape_string(msg->chat->srv->user->nick, -1);
    pattern = g_strdup_printf("\\b%s\\b", nick);
    regex = g_regex_new(pattern, G_REGEX_CASELESS, 0, &err);
    if (!regex){
        ret = RET_ERR(_("g_regex_new() failed: %1$s"), err->message);
        goto FIN;
    }

    ctx.message = msg;
    ctx.pattern = regex;
    ret = srn_markup_renderer_render(markup_renderer,
            msg->rendered_content, &rendered_content, &ctx);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to render markup text: %1$s"), RET_MSG(ret));
        goto FIN;
    }
    if (rendered_content) {
        g_free(msg->rendered_content);
        msg->rendered_content = rendered_content;
    }

FIN:
    if (err) {
        g_error_free(err);
    }
    if (regex){
        g_regex_unref(regex);
    }
    if (pattern){
        g_free(pattern);
    }
    if (nick){
        g_free(nick);
    }

    return ret;
}

static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error){
    int cur_pos;
    struct Context *ctx;
    GString *markup;
    GMatchInfo *match_info;

    cur_pos = 0;
    ctx = srn_markup_renderer_get_user_data(user_data);
    markup = srn_markup_renderer_get_markup(user_data);
    g_regex_match_full(ctx->pattern, text, text_len, 0, 0, &match_info, NULL);
    while(g_match_info_matches(match_info)) {
        int start_pos, end_pos;
        char *escaped_nonmatch, *escaped_match;

        // Mark as mentioned
        ctx->message->mentioned = TRUE;

        // Fetch pos [start_pos, end_pos)
        g_match_info_fetch_pos(match_info, 0, &start_pos, &end_pos);
        escaped_nonmatch = g_markup_escape_text(text + cur_pos, start_pos - cur_pos);
        escaped_match = g_markup_escape_text(text + start_pos, end_pos - start_pos);
        cur_pos = end_pos;

        // Escape non-matched text
        g_string_append(markup, escaped_nonmatch);

        // Highlight and escape matched text
        // TODO: Make this color configurable
        g_string_append(markup, "<span foreground=\"#549ee7\"><b>");
        g_string_append(markup, escaped_match);
        g_string_append(markup, "</b></span>");

        g_free(escaped_nonmatch);
        g_free(escaped_match);

        g_match_info_next(match_info, NULL);
    }
    g_match_info_free(match_info);

    // Deal with the remaining text
    if (cur_pos < text_len) {
        char *escaped_text;
        escaped_text = g_markup_escape_text(text + cur_pos, text_len - cur_pos);
        g_string_append(markup, escaped_text);
        g_free(escaped_text);
    }
}
