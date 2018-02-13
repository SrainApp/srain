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

#ifndef __SUI_PREFS_H
#define __SUI_PREFS_H

#ifndef __IN_SUI_H
	#error This file should not be included directly, include just sui.h
#endif

#include "srain.h"
#include "ret.h"

typedef struct _SuiAppPrefs SuiAppPrefs;
typedef struct _SuiPrefs SuiPrefs;

struct _SuiAppPrefs {
    char *theme;
    // const char *font;
};

struct _SuiPrefs {
    bool notify;
    bool show_topic;
    bool show_avatar;
    bool show_user_list;
    bool preview_image;
    bool render_mirc_color;
};

SuiAppPrefs *sui_app_prefs_new();
SrnRet sui_app_prefs_check(SuiAppPrefs *prefs);
void sui_app_prefs_free(SuiAppPrefs *prefs);

SuiPrefs *sui_prefs_new();
SrnRet sui_prefs_check(SuiPrefs *prefs);
void sui_prefs_free(SuiPrefs *prefs);

#endif /* __SUI_PREFS_H */
