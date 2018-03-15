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

#ifndef __IRC_H
#define __IRC_H

#include <glib.h>
#include <gio/gio.h>

#include "srain.h"

typedef struct _SircSession SircSession;

#define SIRC_SESSION_SSL            1 << 0
#define SIRC_SESSION_SSL_NOTVERIFY  1 << 1
#define SIRC_SESSION_SASL           1 << 2 // Not support yet
#define SIRC_SESSION_IPV6           1 << 3 // Not support yet

#define SIRC_BUF_LEN    513

#define __IN_SIRC_H
#include "sirc_cmd.h"
#include "sirc_event.h"
#include "sirc_numeric.h"
#include "sirc_utils.h"
#include "sirc_config.h"
#undef __IN_SIRC_H

SircSession* sirc_new_session(SircEvents *events, SircConfig *cfg);
void sirc_free_session(SircSession *sirc);
void sirc_connect(SircSession *sirc, const char *host, int port);
void sirc_cancel_connect(SircSession *sirc);
void sirc_disconnect(SircSession *sirc);
int sirc_get_fd(SircSession *sirc);
GIOStream* sirc_get_stream(SircSession *sirc);
SircEvents* sirc_get_events(SircSession *sirc);
void* sirc_get_ctx(SircSession *sirc);
void sirc_set_ctx(SircSession *sirc, void *ctx);

#endif /* __IRC_H */
