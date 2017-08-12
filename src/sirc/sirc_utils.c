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

#include <strings.h>
#include <glib.h>

#include "srain.h"

bool sirc_nick_cmp(const char *nick1, const char *nick2){
    g_return_val_if_fail(nick1, FALSE);
    g_return_val_if_fail(nick2, FALSE);

    return g_ascii_strcasecmp(nick1, nick2) == 0;
}

bool sirc_prefix_is_server(const char *prefix){
    return NULL;
}

/* TODO */
const char* sirc_prefix_get_nick(const char *prefix){
    return NULL;
}

const char* sirc_prefix_get_host(const char *prefix){
    return NULL;
}

const char* sirc_prefix_get_user(const char *prefix){
    return NULL;
}
