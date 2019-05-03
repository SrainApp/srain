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
#include <glib.h>
#include <string.h>

#include "srain.h"
#include "log.h"
#include "i18n.h"
#include "markup_renderer.h"

#include "render/render.h"
#include "./renderer.h"
#include "./mirc.h"

#define MAX_CTX_STACK_SIZE 128

typedef struct _ColorlizeContext {
    int stack[MAX_CTX_STACK_SIZE];
    int ptr; // Stack pointer
    unsigned fg_color;
    unsigned bg_color;
    GString *str;
} ColorlizeContext;

static void init(void);
static void finalize(void);
static SrnRet render(SrnMessage *msg);
static void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error);
static void do_colorize(ColorlizeContext *ctx, char ch);

/**
 * @brief mirc_strip_renderer is a render moduele for rendering mIRC color in
 * message.
 *
 * ref: https://en.wikichip.org/wiki/irc/colors
 */
SrnMessageRenderer mirc_colorize_renderer = {
    .name = "mirc_colorize",
    .init = init,
    .finalize = finalize,
    .render = render,
};

// TODO: define in theme CSS?
static const char *color_map[] = {
    [MIRC_COLOR_WHITE]          = "#FFFFFF",
    [MIRC_COLOR_BLACK]          = "#000000",
    [MIRC_COLOR_NAVY]           = "#00007F",
    [MIRC_COLOR_GREEN]          = "#009300",
    [MIRC_COLOR_RED]            = "#FF0000",
    [MIRC_COLOR_MAROON]         = "#7F0000",
    [MIRC_COLOR_PURPLE]         = "#9C009C",
    [MIRC_COLOR_OLIVE]          = "#FC7F00",
    [MIRC_COLOR_YELLOW]         = "#FFFF00",
    [MIRC_COLOR_LIGHT_GREEN]    = "#00FC00",
    [MIRC_COLOR_TEAL]           = "#009393",
    [MIRC_COLOR_CYAN]           = "#00FFFF",
    [MIRC_COLOR_ROYAL_BLUE]     = "#0000FC",
    [MIRC_COLOR_MAGENTA]        = "#FF00FF",
    [MIRC_COLOR_GRAY]           = "#7F7F7F",
    [MIRC_COLOR_LIGHT_GRAY]     = "#D2D2D2",
    [MIRC_COLOR_UNKNOWN]        = "", // Preventing out of bound
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

void text(GMarkupParseContext *context, const gchar *text,
        gsize text_len, gpointer user_data, GError **error) {
    GString *str;
    ColorlizeContext *ctx;

    str = g_string_new(NULL);
    ctx = g_malloc0(sizeof(ColorlizeContext));
    ctx->fg_color = MIRC_COLOR_UNKNOWN;
    ctx->bg_color = MIRC_COLOR_UNKNOWN;
    ctx->str = srn_markup_renderer_get_markup(user_data);

    for (int i = 0; i < text_len; i++){
        switch (text[i]){
            case MIRC_COLOR:
                {
                    /* Format: "\30[fg_color],[bg_color]",
                     * 0 <= length of fg_color or bg_color <= 2*/
                    const char *startptr = &text[i] + 1;
                    char *endptr = NULL;
                    bool has_fg_color = FALSE;
                    bool has_bg_color = FALSE;
                    unsigned fg_color = strtoul(startptr, &endptr, 10);
                    if (endptr > startptr){ // Get foreground color
                        has_fg_color = TRUE;
                        while (endptr - startptr > 2){ // Wrong number of digits
                            fg_color = fg_color / 10;
                            endptr--;
                        }
                        DBG_FR("Get foreground color: %u", fg_color);
                        ctx->fg_color = fg_color;
                    }
                    i += endptr - startptr;
                    if (*endptr == ',') { // background color exists
                        endptr++;
                        startptr = endptr;
                        endptr = NULL;
                        unsigned bg_color = strtoul(startptr, &endptr, 10);
                        if (endptr > startptr){ // Get background color
                            has_bg_color = TRUE;
                            while (endptr - startptr > 2){ // Wrong number of digits
                                bg_color = bg_color / 10;
                                endptr--;
                            }
                            DBG_FR("Get background color: %u", bg_color);
                            ctx->bg_color = bg_color;
                        }
                        i += endptr - startptr;
                    }
                    if (!has_fg_color && !has_bg_color) { // Clear previous color
                        ctx->fg_color = MIRC_COLOR_UNKNOWN;
                        ctx->bg_color = MIRC_COLOR_UNKNOWN;
                    }
                    do_colorize(ctx, MIRC_COLOR);
                    break;
                }
            case MIRC_BOLD:
            case MIRC_ITALICS:
            case MIRC_UNDERLINE:
            case MIRC_BLINK:
            case MIRC_REVERSE:
            case MIRC_PLAIN:
                do_colorize(ctx, text[i]);
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

    // Close all unclosed tags
    do_colorize(ctx, MIRC_PLAIN);
    g_free(ctx);
}

static void do_colorize(ColorlizeContext *ctx, char ch){
    int ptr;
    bool open_tag = TRUE;

    ptr = ctx->ptr - 1;

    if (ch == MIRC_PLAIN){
        DBG_FR("Reset all format");
        // Reset color
        ctx->fg_color = MIRC_COLOR_UNKNOWN;
        ctx->bg_color = MIRC_COLOR_UNKNOWN;
        // Clear stack
        while (ptr >= 0){
            do_colorize(ctx, ctx->stack[ptr]);
            ptr--;
        }
        return;
    }

    while (ptr >= 0){
        if (ctx->stack[ptr] == ch){
            open_tag = FALSE;
            break;
        }
        ptr--;
    }

    if (open_tag) {
        DBG_FR("Opening tag: 0x%x", ch);

        ctx->stack[ctx->ptr++] = ch;
        if (ctx->ptr >= MAX_CTX_STACK_SIZE){
            ERR_FR("Too many tags in stack, abort");
            return;
        }

        switch (ch){
            case MIRC_BOLD:
                g_string_append(ctx->str, "<b>");
                break;
            case MIRC_ITALICS:
                g_string_append(ctx->str, "<i>");
                break;
            case MIRC_UNDERLINE:
                g_string_append(ctx->str, "<u>");
                break;
            case MIRC_REVERSE:
                // TODO: Not supported yet
                break;
            case MIRC_BLINK:
                // TODO: Not supported yet
                break;
            case MIRC_COLOR:
                if (ctx->fg_color > MIRC_COLOR_UNKNOWN){
                    WARN_FR("Invalid mirc foreground color: %u", ctx->fg_color);
                    ctx->fg_color = MIRC_COLOR_UNKNOWN;
                }
                if (ctx->bg_color > MIRC_COLOR_UNKNOWN){
                    WARN_FR("Invalid mirc background color: %u", ctx->bg_color);
                    ctx->bg_color = MIRC_COLOR_UNKNOWN;
                }
                if (ctx->fg_color == MIRC_COLOR_UNKNOWN) {
                    if (ctx->bg_color == MIRC_COLOR_UNKNOWN) {
                        ctx->ptr--; // Default color, tag can be omitted
                    } else {
                        g_string_append_printf(ctx->str,
                                "<span background=\"%s\">",
                                color_map[ctx->bg_color]);
                    }
                } else {
                    g_string_append_printf(ctx->str,
                            "<span foreground=\"%s\">",
                            color_map[ctx->fg_color]);
                }
                break;
        }
    } else {
        DBG_FR("Closeing tag: 0x%x", ch);

        // Closes all unclosed tags before the start tag corresponding to current tag
        for (int i = ctx->ptr - 1; i >= ptr; i--){
            DBG_FR("Tag: 0x%x will be closed because of 0x%x", ctx->stack[i], ch);
            switch (ctx->stack[i]){
                case MIRC_BOLD:
                    g_string_append(ctx->str, "</b>");
                    break;
                case MIRC_ITALICS:
                    g_string_append(ctx->str, "</i>");
                    break;
                case MIRC_UNDERLINE:
                    g_string_append(ctx->str, "</u>");
                    break;
                case MIRC_REVERSE:
                    // TODO: Not supported yet
                    break;
                case MIRC_BLINK:
                    // TODO: Not supported yet
                    break;
                case MIRC_COLOR:
                    g_string_append(ctx->str, "</span>");
                    break;
            }
        }
        int tmp_ptr = ctx->ptr;
        ctx->ptr = ptr;
        // Reopen the closed tag excepting the current tag
        for (int i = ptr + 1; i < tmp_ptr; i++){
            do_colorize(ctx, ctx->stack[i]);
        }

        if (ch == MIRC_COLOR) {
            if (ctx->fg_color == MIRC_COLOR_UNKNOWN && ctx->bg_color == MIRC_COLOR_UNKNOWN ){
                DBG_FR("Tag 0x%x closed with default {fore,back}ground color", ch);
            } else {
                DBG_FR("Reopening tag 0x%x because foreground color = %u and background = %u",
                        ch, ctx->fg_color, ctx->bg_color);
                // One or more of fg_color and bg_color is not default color,
                // we should open a new tag for showing them.
                do_colorize(ctx, MIRC_COLOR);
            }
        }
    }
}
