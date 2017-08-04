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

#ifndef __UTILS_H
#define __UTILS_H

#include <time.h>
#include <srain.h>

unsigned long get_time_since_first_call_ms(void);
time_t get_current_time_s(void);
void time_to_str(time_t time, char *timestr, size_t size, const char *fmt);
void str_assign(char **left, const char *right);
bool str_is_empty(const char *str);

#endif /* __UTILS_H */
