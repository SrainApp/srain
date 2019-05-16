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

#include <glib.h>

#include "srain.h"
#include "log.h"

#include "filter/filter.h"
#include "./filter.h"

// Bits of a SrnRenderFlags(int)
#define MAX_FILTER  sizeof(SrnFilterFlags) * 8

extern SrnMessageFilter user_filter;
extern SrnMessageFilter regex_filter;
extern SrnMessageFilter log_filter;
static SrnMessageFilter *filters[MAX_FILTER];

void srn_filter_init(void){
    int i;

    /* NOTE: Do not change the order filterer . */
    i = 0;
    filters[i++] = &user_filter;
    filters[i++] = &regex_filter;
    filters[i++] = &log_filter;
    g_warn_if_fail(i < MAX_FILTER);

    /* Initial all filters */
    for (int i = 0; i < MAX_FILTER; i++){
        if (!filters[i] || !filters[i]->init) {
            continue;
        }
        filters[i]->init();
    }
}

void srn_filter_finalize(void){
    /* Finalize all filters */
    for (int i = 0; i < MAX_FILTER; i++){
        if (!filters[i] || !filters[i]->finalize) {
            continue;
        }
        filters[i]->finalize();
    }
}

bool srn_filter_message(const SrnMessage *msg, SrnFilterFlags flags){
    g_return_val_if_fail(msg, SRN_ERR);

    for (int i = 0; i < MAX_FILTER; i++){
        if (!(flags & (1 << i))) {
            continue;
        }
        g_return_val_if_fail(filters[i]
                && filters[i]->name
                && filters[i]->filter, SRN_ERR);

        if (!filters[i]->filter(msg)) {
            return FALSE;
        }
    }

    return TRUE;
}
