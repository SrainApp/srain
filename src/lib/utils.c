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
 * @file utils.c
 * @brief Utility functions
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-04-22
 */

#include <glib.h>
#include <string.h>

#include "srain.h"
#include "log.h"

/* Datetime utilities */

/**
 * @brief Get the time since this function firstly called, in millsceonds
 *
 * @return 
 *
 * NOTE: This function is not thread-safe
 **/
unsigned long get_time_since_first_call_ms(void){
    static gint64 first = 0;
    unsigned long ret;

    if (first == 0){
        first = g_get_monotonic_time() / 1000;
        ret = 0;
    } else {
        ret = (g_get_monotonic_time() / 1000) - first;
    }

    return ret;
}

void time_to_str(time_t time, char *timestr, size_t size, const char *fmt){
    strftime(timestr, size - 1, fmt, localtime(&time));

    timestr[size-1] = '\0';
}

void str_assign(char **left, const char *right){
    if (*left) {
        g_free(*left);
    }
    *left = g_strdup(right);
}

bool str_is_empty(const char *str){
    if (!str) {
        return TRUE;
    }
    while (*str) {
        if (!g_ascii_isspace(*str++)){
            return FALSE;
        }
    }
    return TRUE;
}

void str_transcoding(char **str, const char *from_codeset){
    if (!*str) return;

    if (g_ascii_strcasecmp(from_codeset, SRN_CODESET) == 0) {
        // UTF-8 to UTF-8, just make sure it is valid
        if (g_utf8_validate(*str, -1, NULL)) {
            return;
        }
        // If invalid, make it valid
        str_assign(str, g_utf8_make_valid(*str, -1));
        return;
    }

    // To other codeset
    GError *err = NULL;
    char *tmp = g_convert_with_fallback(*str, -1, SRN_CODESET, from_codeset, "�", NULL, NULL, &err);
    if (tmp){
        str_assign(str, tmp);
    }
    if (err) {
        WARN_FR("Failed to convert line from %s to %s: %s", from_codeset, SRN_CODESET, err->message);
        g_error_free(err);
    }
}
