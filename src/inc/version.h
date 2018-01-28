/* Copyright (C) 2016-2018 Shengyu Zhang <srain@srain.im>
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

#ifndef __VERSION_H
#define __VERSION_H

#include "ret.h"

typedef struct _SrnVersion SrnVersion;

struct _SrnVersion {
    char *raw;
    unsigned int major;
    unsigned int minor;
    unsigned int micro; // a.k.a patch
    char *build;
};

SrnVersion *srn_version_new(const char *raw);
SrnRet srn_version_parse(SrnVersion *ver);
void srn_version_free(SrnVersion *ver);

#endif /* __VERSION_H */
