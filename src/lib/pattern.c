/* Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
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
 * @file pattern.c
 * @brief This file provides convenient interface for regex pattern management.
 * Refer to https://developer.gnome.org/glib/stable/glib-regex-syntax.html
 * for pattern syntax.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version
 * @date 2019-05-18
 */

#include <glib.h>

#include "ret.h"
#include "i18n.h"

static GHashTable *pattern_table;

void srn_pattern_init(void) {
    pattern_table = g_hash_table_new_full(
            g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_regex_unref);
}

SrnRet srn_pattern_add_pattern(const char *name, const char *pattern) {
    GError *err;
    GRegex *regex;

    if (g_hash_table_contains(pattern_table, name)) {
        return SRN_ERR;
    }

    err = NULL;
    regex = g_regex_new(pattern, 0, 0, &err);
    if (err) {
        return RET_ERR("%s", err->message);
    }
    g_hash_table_insert(pattern_table, g_strdup(name), regex);

    return SRN_OK;
}

GRegex *srn_pattern_get_regex(const char *name) {
    return g_hash_table_lookup(pattern_table, name);
}

SrnRet srn_pattern_rm_pattern(const char *name) {
    return g_hash_table_remove(pattern_table, name) ? SRN_OK : SRN_ERR;
}

/**
 * @brief srn_pattern_list_pattern lists name of all available patterns.
 *
 * @return A GList which contains constant string.
 * The contained string MUST not be freed by user.
 * The GList itself should be freed by user via g_list_free().
 */
GList* srn_pattern_list_pattern() {
    gpointer key;
    GList *lst;
    GHashTableIter iter;

    lst = NULL;
    g_hash_table_iter_init(&iter, pattern_table);
    while (g_hash_table_iter_next(&iter, &key, NULL)) {
        lst = g_list_append(lst, key);
    }

    return lst;
}

void srn_pattern_finalize(void) {
    g_hash_table_destroy(pattern_table);
}
