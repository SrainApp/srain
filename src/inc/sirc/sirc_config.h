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

#ifndef __SIRC_CONFIG_H
#define __SIRC_CONFIG_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include "srain.h"
#include "ret.h"

typedef struct _SircConfig SircConfig;

struct _SircConfig {
    bool tls;
    bool tls_noverify;
    // bool ipv6;
    // bool sasl;
    char *encoding;
};

SircConfig* sirc_config_new();
SrnRet sirc_config_check(SircConfig *cfg);
char* sirc_config_dump(SircConfig *cfg);
void sirc_config_free(SircConfig *cfg);

#endif /* __SIRC_CONFIG_H */
