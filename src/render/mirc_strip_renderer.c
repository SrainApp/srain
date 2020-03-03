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

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "srain.h"
#include "log.h"
#include "i18n.h"
#include "markup_renderer.h"

#include "./renderer.h"
#include "./mirc.h"

static void init(void);
static void finalize(void);
static SrnRet render(SrnMessage *msg);
static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);

/**
 * @brief mirc_strip_renderer is a render moduele for strip mIRC color from
 * message.
 *
 * ref: https://en.wikichip.org/wiki/irc/colors
 */
SrnMessageRenderer mirc_strip_renderer = {
    .name = "mirc_strip",
    .init = init,
    .finalize = finalize,
    .render = render,
};

static SrnMarkupRenderer *markup_renderer;

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
    char *rendered_content;
    SrnRet ret;

    rendered_content = NULL;
    ret = srn_markup_renderer_render(markup_renderer,
            msg->rendered_content, &rendered_content, msg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Failed to render markup text: %1$s"), RET_MSG(ret));
    }
    if (rendered_content) {
        g_free(msg->rendered_content);
        msg->rendered_content = rendered_content;
    }

    return SRN_OK;
}

static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error){
    GString *str;

    str = srn_markup_renderer_get_markup(user_data);

    for (int i = 0; i < text_len; i++){
        switch (text[i]){
            case MIRC_COLOR:
                {
                    const char *startptr = &text[i] + 1;
                    char *endptr = NULL;
                    strtoul(startptr, &endptr, 10);
                    if (endptr > startptr){ // Get foreground color
                        while (endptr - startptr > 2){ // Wrong number of digits
                            endptr--;
                        }
                    }
                    i += endptr - startptr;
                    if (*endptr == ',') { // background color exists
                        endptr++;
                        startptr = endptr;
                        endptr = NULL;
                        strtoul(startptr, &endptr, 10);
                        if (endptr > startptr){ // Get background color
                            while (endptr - startptr > 2){ // Wrong number of digits
                                endptr--;
                            }
                        }
                        i += endptr - startptr;
                    }
                    break;
                }
            case MIRC_BOLD:
            case MIRC_ITALICS:
            case MIRC_UNDERLINE:
            case MIRC_BLINK:
            case MIRC_REVERSE:
            case MIRC_PLAIN:
                break;
            default:
                {
                    // No control character, it is a utf-8 sequence
                    const char *next = g_utf8_next_char(&text[i]);
                    char *escape = g_markup_escape_text(&text[i], next - &text[i]);
                    str = g_string_append(str, escape);
                    g_free(escape);
                    i += next - &text[i] - 1;
                    break;
                }
        }
    }
}
