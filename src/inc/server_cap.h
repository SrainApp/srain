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

#ifndef __SERVER_CAP_H
#define __SERVER_CAP_H

#include "srain.h"
#include "ret.h"

typedef struct _ServerCap ServerCap;

struct _ServerCap {
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

ServerCap* server_cap_new();
void server_cap_free(ServerCap *scap);
SrnRet server_cap_enable(ServerCap *scap, const char *name, bool enable);
bool server_cap_is_support(ServerCap *scap, const char *name, const char *value);
char* server_cap_dump(ServerCap *scap);

#endif /* __SERVER_CAP_H */
