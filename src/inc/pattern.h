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
 * @file pattern.h
 * @brief Simple pattern management.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 
 * @date 2019-05-16
 */

#ifndef __PATTERN_H
#define __PATTERN_H

#include <glib.h>
#include "ret.h"

void srn_pattern_init(void);
void srn_pattern_finalize(void);

SrnRet srn_pattern_add_pattern(const char *name, const char *pattern);
SrnRet srn_pattern_rm_pattern(const char *name);
GRegex* srn_pattern_get_regex(const char *name);
GList* srn_pattern_list_pattern();

#endif /* __PATTERN_H */
