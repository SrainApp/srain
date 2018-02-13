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

#ifndef __SIRC_UTILS_H
#define  __SIRC_UTILS_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include "srain.h"

#define CHAN_PREFIX1    '#'
#define CHAN_PREFIX2    '&'

#define sirc_is_chan(ch) (ch && (ch[0] == CHAN_PREFIX1 || ch[0] == CHAN_PREFIX2))

bool sirc_nick_cmp(const char *nick1, const char *nick2);
bool sirc_prefix_is_server(const char *prefix);

const char* sirc_prefix_get_nick(const char *prefix);
const char* sirc_prefix_get_host(const char *prefix);
const char* sirc_prefix_get_user(const char *prefix);

#endif /* __SIRC_UTILS_H */
