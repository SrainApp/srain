/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @file mirc_strip.c
 * @brief mIRC strip decorator
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-05-06
 */

#include <string.h>
#include <glib.h>

#include "decorator.h"
#include "mirc.h"

#include "srain.h"
#include "log.h"

static char *mirc_stirp(Message *msg, int index, const char *frag);

Decorator mirc_strip_decroator = {
    .name = "mirc_strip",
    .func = mirc_stirp,
};

static char *mirc_stirp(Message *msg, int index, const char *frag){
    int j;
    int len;
    char *dfrag;
    GString *str;

    str = g_string_new(NULL);

    j = 0;
    len = strlen(frag);

    for (int i = 0; i < len; i++){
        switch (frag[i]){
            case MIRC_COLOR:
                {
                    const char *startptr = &frag[i] + 1;
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
                    const char *next = g_utf8_next_char(&frag[i]);
                    char *escape = g_markup_escape_text(&frag[i], next - &frag[i]);
                    str = g_string_append(str, escape);
                    g_free(escape);
                    i += next - &frag[i] - 1;
                    break;
                }
        }
    }

    dfrag = str->str;
    g_string_free(str, FALSE);

    return dfrag;
}
