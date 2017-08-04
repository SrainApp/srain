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
 * @file filter.c
 * @brief Filter is a mechanism for filtering XML fromatted message in
 *        flow style
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-03-16
 */

#include <glib.h>
#include <string.h>

#include "filter.h"

#include "srain.h"
#include "log.h"

#define MAX_FILTER   32  // Bits of a FilterFlag(int)

extern Filter nick_filter;
extern Filter regex_filter;
extern Filter chat_log_filter;

static Filter *filters[MAX_FILTER];

void filter_init(){
    memset(filters, 0, sizeof(filters));

    filters[0] = &nick_filter;
    filters[1] = &regex_filter;
    filters[2] = &chat_log_filter;
}

bool filter_message(const Message *msg, FilterFlag flag, void *user_data){
    int ret;

    for (int i = 0; i < MAX_FILTER; i++){
        if ((flag & (1 << i))
                && filters[i]
                && filters[i]->name
                && filters[i]->func){

            DBG_FR("Run filter '%s' for message %p", filters[i]->name, msg);

            ret = filters[i]->func(msg, flag, user_data);
            if (!ret){
                DBG_FR("Filter '%s' blocked message %p", filters[i]->name, msg);

                break;
            }
        }
    }

    return ret;
}
