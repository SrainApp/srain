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
 * @file extra_data.c
 * @brief This file provides a in-memory KV storage which can custom
 * destructors for each key.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 
 * @date 2019-05-25
 */

#include <glib.h>

#include "extra_data.h"

struct _SrnExtraData {
    GHashTable *data_table;
    GHashTable *destory_func_table;
};

SrnExtraData* srn_extra_data_new(void) {
    SrnExtraData *self;

    self = g_malloc0(sizeof(SrnExtraData));
    self->data_table = g_hash_table_new(g_str_hash, g_str_equal);
    self->destory_func_table = g_hash_table_new(g_str_hash, g_str_equal);

    return self;
}

void srn_extra_data_free(SrnExtraData *self) {
    gpointer key, val;
    GHashTableIter iter;

    // Free all extra data via destory func
    g_hash_table_iter_init(&iter, self->data_table);
    while (g_hash_table_iter_next(&iter, &key, &val)){
        GDestroyNotify func;

        func = g_hash_table_lookup(self->data_table, key);
        if (func) {
            func(val);
        }
    }

    g_hash_table_destroy(self->data_table);
    g_hash_table_destroy(self->destory_func_table);
}

void* srn_extra_data_get(SrnExtraData *self, const char *key) {
    return g_hash_table_lookup(self->data_table, key);
}

void srn_extra_data_set(SrnExtraData *self, const char *key, void *val,
        GDestroyNotify val_destory_func) {
    g_return_if_fail(key);

    if (val) { // Add a key, NOTE: Update a exsting key is not allowed for now
        g_return_if_fail(!g_hash_table_contains(self->data_table, key));
        g_return_if_fail(!g_hash_table_contains(self->destory_func_table, key));

        g_hash_table_insert(self->data_table, (gpointer)key, val);
        g_hash_table_insert(self->destory_func_table, (gpointer)key,
                val_destory_func);
    } else { // Remove a key
        void *oldval;
        GDestroyNotify func;

        g_return_if_fail(g_hash_table_contains(self->data_table, key));
        g_return_if_fail(g_hash_table_contains(self->destory_func_table, key));

        oldval = g_hash_table_lookup(self->data_table, key);
        func = g_hash_table_lookup(self->destory_func_table, key);
        if (func) {
            func(oldval);
        }
        g_hash_table_remove(self->data_table, key);
        g_hash_table_remove(self->destory_func_table, key);
    }
}
