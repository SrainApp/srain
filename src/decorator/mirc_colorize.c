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

/**
 * @file mirc_colorize.c
 * @brief mIRC colorize decorator
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-08-27
 *
 * ref: https://en.wikichip.org/wiki/irc/colors
 */

#include <stdlib.h>
#include <glib.h>
#include <string.h>

#include "decorator.h"
#include "mirc.h"

#include "srain.h"
#include "log.h"

#define MAX_CTX_STACK_SIZE 128

typedef struct _ColorlizeContext {
    int stack[MAX_CTX_STACK_SIZE];
    int ptr; // Stack pointer
    unsigned fg_color;
    unsigned bg_color;
    GString *str;
} ColorlizeContext;

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

static char *mirc_colorize(Message *msg, int index, const char *frag);
static void do_colorize(ColorlizeContext *ctx, char ch);

Decorator mirc_colorize_decroator = {
    .name = "mirc_colorize",
    .func = mirc_colorize,
};

static char *mirc_colorize(Message *msg, int index, const char *frag){
    int len;
    char *dfrag;
    GString *str;
    ColorlizeContext *ctx;

    len = strlen(frag);
    str = g_string_new(NULL);
    ctx = g_malloc0(sizeof(ColorlizeContext));
    ctx->fg_color = MIRC_COLOR_UNKNOWN;
    ctx->bg_color = MIRC_COLOR_UNKNOWN;
    ctx->str = str;

    for (int i = 0; i < len; i++){
        switch (frag[i]){
            case MIRC_COLOR:
                {
                    /* Format: "\30[fg_color],[bg_color]",
                     * 0 <= length of fg_color or bg_color <= 2*/
                    const char *startptr = &frag[i] + 1;
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
                do_colorize(ctx, frag[i]);
                break;
            default:
                {
                    // No control character, it is a utf-8 sequence
                    const char *next = g_utf8_next_char(&frag[i]);
                    char *escape = g_markup_escape_text(&frag[i], next - &frag[i]);
                    str = g_string_append(str, escape);
                    g_free(escape);
                    i += next - &frag[i] - 1;
                    break;
                }
        }
    }

    // Close all unclosed tags
    do_colorize(ctx, MIRC_PLAIN);

    dfrag = ctx->str->str;
    g_string_free(ctx->str, FALSE);
    g_free(ctx);

    return dfrag;
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
                ctx->str = g_string_append(ctx->str, "<b>");
                break;
            case MIRC_ITALICS:
                ctx->str = g_string_append(ctx->str, "<i>");
                break;
            case MIRC_UNDERLINE:
                ctx->str = g_string_append(ctx->str, "<u>");
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
                    ctx->str = g_string_append(ctx->str, "</b>");
                    break;
                case MIRC_ITALICS:
                    ctx->str = g_string_append(ctx->str, "</i>");
                    break;
                case MIRC_UNDERLINE:
                    ctx->str = g_string_append(ctx->str, "</u>");
                    break;
                case MIRC_REVERSE:
                    // TODO: Not supported yet
                    break;
                case MIRC_BLINK:
                    // TODO: Not supported yet
                    break;
                case MIRC_COLOR:
                    ctx->str = g_string_append(ctx->str, "</span>");
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
