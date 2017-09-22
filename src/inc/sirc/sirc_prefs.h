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

#ifndef __SIRC_PREFS_H
#define __SIRC_PREFS_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include "srain.h"
#include "ret.h"

typedef struct _SircPrefs SircPrefs;

struct _SircPrefs {
    bool tls;
    bool tls_noverify;
    // bool ipv6;
    // bool sasl;
};

SircPrefs *sirc_prefs_new();
SrnRet sirc_prefs_check(SircPrefs *prefs);
char* sirc_prefs_dump(SircPrefs *prefs);
void sirc_prefs_free(SircPrefs *prefs);

#endif /* __SIRC_PREFS_H */
