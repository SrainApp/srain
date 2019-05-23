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

#ifndef __FILTER_EXTRA_H
#define __FILTER_EXTRA_H

#include "core/core.h"

SrnRet srn_pattern_filter_add_pattern(SrnChat *chat, const char *pattern);
SrnRet srn_pattern_filter_rm_pattern(SrnChat *chat, const char *pattern);

#endif /* __FILTER_EXTRA_H */
