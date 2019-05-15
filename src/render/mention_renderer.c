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

#include "core/core.h"
#include "markup_renderer.h"
#include "i18n.h"

#include "./renderer.h"

static void init(void);
static void finalize(void);
static SrnRet render(SrnMessage *msg);
static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);

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
    parser->start_element = NULL;
    parser->end_element = NULL;
    parser->text = text;
}

void finalize(void) {
    srn_markup_renderer_free(markup_renderer);
}

SrnRet render(SrnMessage *msg) {
    char *nick = NULL;
    char *pattern = NULL;
    char *raw_content = NULL;
    GError *err = NULL;
    GRegex *regex = NULL;
    GMatchInfo *match_info = NULL;
    SrnRet ret = SRN_OK;

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

    raw_content = NULL;
    ret = srn_markup_renderer_render(markup_renderer,
            msg->rendered_content, &raw_content, NULL);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to render markup text: %1$s"), RET_MSG(ret));
        goto FIN;
    }
    msg->mentioned = g_regex_match(regex, raw_content, 0, &match_info);

FIN:
    if (raw_content) {
        g_free(raw_content);
    }
    if (err) {
        g_error_free(err);
    }
    if (match_info){
        g_match_info_free(match_info);
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
    g_string_append_len(srn_markup_renderer_get_markup(user_data), text, text_len);
}
