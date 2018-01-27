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

#ifndef __SUI_EVENT_H
#define __SUI_EVENT_H

#include <glib.h>

#include "srain.h"

#ifndef __IN_SUI_H
	#error This file should not be included directly, include just sui.h
#endif

#define SUI_EVENT_PARAM_BOOL    "b"
#define SUI_EVENT_PARAM_INT     "i"
#define SUI_EVENT_PARAM_STRING  "&s"
#define SUI_EVENT_PARAM_STRINGS "^a&s"

typedef enum {
    SUI_EVENT_OPEN = 0,
    SUI_EVENT_ACTIVATE,
    SUI_EVENT_CONNECT,
    SUI_EVENT_DISCONNECT,
    SUI_EVENT_QUIT,
    SUI_EVENT_SEND,
    SUI_EVENT_JOIN,
    SUI_EVENT_PART,
    SUI_EVENT_QUERY,
    SUI_EVENT_UNQUERY,
    SUI_EVENT_KICK,
    SUI_EVENT_INVITE,
    SUI_EVENT_WHOIS,
    SUI_EVENT_IGNORE,
    SUI_EVENT_CUTOVER,
    SUI_EVENT_SERVER_LIST,
    SUI_EVENT_CHAN_LIST,
    SUI_EVENT_UNKNOWN,
} SuiEvent;

typedef int (*SuiAppEventCallback) (SuiEvent event, GVariantDict *params);

typedef int (*SuiEventCallback) (SuiSession *sui, SuiEvent event, GVariantDict *params);

typedef struct {
    SuiEventCallback disconnect;
    SuiEventCallback quit;
    SuiEventCallback send;
    SuiEventCallback join;
    SuiEventCallback part;
    SuiEventCallback query;
    SuiEventCallback unquery;
    SuiEventCallback kick;
    SuiEventCallback invite;
    SuiEventCallback whois;
    SuiEventCallback ignore;
    SuiEventCallback cutover;
    SuiEventCallback chan_list;
} SuiEvents;

typedef struct {
    SuiAppEventCallback open;
    SuiAppEventCallback activate;
    SuiAppEventCallback connect;
    SuiAppEventCallback server_list;
} SuiAppEvents;

#endif /* __SUI_EVENT_H */
