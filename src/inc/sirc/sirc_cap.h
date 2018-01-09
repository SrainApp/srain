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

#ifndef __SIRC_CAP_H
#define __SIRC_CAP_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include "ret.h"

typedef struct _SircCap SircCap;
typedef struct _SircCapSupport SircCapSupport;

struct _SircCap {
    /* Capabilities */
    // Version 3.1
    bool identify_msg;
    bool mulit_prefix;
    bool away_notify;
    bool account_notify;
    bool extended_join;
    bool sasl;

    // Version 3.2
    bool server_time;
    bool userhost_in_names;
    bool cap_notify;
    bool chghost;

    // Vendor-Specific
    bool znc_server_time_iso;
    bool znc_server_time;
};

struct _SircCapSupport {
    const char *name;
    ptrdiff_t offset;
    bool (*is_support)(const char *);
};

SircCap* sirc_cap_new();
void sirc_cap_free(SircCap *scap);
SrnRet sirc_cap_enable(SircCap *scap, const char *name, bool enable);
bool sirc_cap_is_support(SircCap *scap, const char *name, const char *value);
char* sirc_cap_dump(SircCap *scap);

#endif /* __SIRC_CAP_H */
