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

#ifndef __SRAIN_H
#define __SRAIN_H

#include <stdint.h>
#include <glib.h>

#if defined __STDC_VERSION__ && __STDC_VERSION__ <= 201710L
typedef gboolean bool;
#endif

/* General result value */
#define SRN_OK      0
#define SRN_ERR    -1
#define SRN_EAGAIN -2
#define SRN_EEXIST -3

#define SRN_TRUE    TRUE
#define SRN_FALSE   FALSE

#define SRN_CODESET "UTF-8"

#endif /* __SRAIN_H */
