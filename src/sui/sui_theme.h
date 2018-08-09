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

#ifndef __SUI_THEME_H
#define __SUI_THEME_H

#include "ret.h"

typedef struct _SuiThemeManager SuiThemeManager;

SuiThemeManager* sui_theme_manager_new();
void sui_theme_manager_free(SuiThemeManager *self);
SrnRet sui_theme_manager_apply(SuiThemeManager *self, const char *theme);

#endif /* __SUI_THEME_H */
