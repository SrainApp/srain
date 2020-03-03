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

bool sirc_target_equal(const char *t1, const char *t2);
bool sirc_target_is_servername(SircSession *sirc, const char *target);
bool sirc_target_is_nickname(SircSession *sirc, const char *target);
bool sirc_target_is_service(SircSession *sirc, const char *target);
bool sirc_target_is_channel(SircSession *sirc, const char *target);

#endif /* __SIRC_UTILS_H */
