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
 * @file pattern_set.h
 * @brief Simple pattern set management.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 
 * @date 2019-05-16
 */

#ifndef __PATTERN_SET_H
#define __PATTERN_SET_H

#include <glib.h>
#include "ret.h"

typedef struct _SrnPatternSet SrnPatternSet;

SrnPatternSet* srn_pattern_set_new(void);
void srn_pattern_set_free(SrnPatternSet *self);

SrnRet srn_pattern_set_add(SrnPatternSet *self, const char *name, const char *pattern);
SrnRet srn_pattern_set_rm(SrnPatternSet *self, const char *name);
GRegex* srn_pattern_set_get(SrnPatternSet *self, const char *name);
GList* srn_pattern_set_list(SrnPatternSet *self);

#endif /* __PATTERN_SET_H */
