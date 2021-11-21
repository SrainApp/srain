/*
 * Copyright (C) 2004-2012 George Yunaev <gyunaev@ulduzsoft.com>
 * Copyright (C) 2017 Shengyu Zhang <silverainz@outlook.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

/* This file is originated from libircclient(include/libirc_events.h) */

#ifndef __SIRC_EVENT_H
#define __SIRC_EVENT_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

typedef void (*SircSimpleEventCallback) (SircSession *sirc, const char *event);

typedef void (*SircEventCallback) (SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

typedef void (*SircNumericEventCallback) (SircSession *sirc, int event,
        const char *origin, const char *params[], int count);

typedef struct {
    SircSimpleEventCallback     connect;
    SircEventCallback           connect_fail;
    SircEventCallback           disconnect;

    SircNumericEventCallback    welcome;
    SircEventCallback           nick;
    SircEventCallback           quit;
    SircEventCallback           join;
    SircEventCallback           part;
    SircEventCallback           mode;
    SircEventCallback           umode;
    SircEventCallback           topic;
    SircEventCallback           kick;
    SircEventCallback           channel;
    SircEventCallback           privmsg;
    SircEventCallback           notice;
    SircEventCallback           tagmsg;
    SircEventCallback           channel_notice;
    SircEventCallback           invite;
    SircEventCallback           ctcp_req;
    SircEventCallback           ctcp_rsp;
    SircEventCallback           cap;
    SircEventCallback           authenticate;
    SircEventCallback           ping;
    SircEventCallback           pong;
    SircEventCallback           error;
    SircEventCallback           unknown;

    SircNumericEventCallback    numeric;
} SircEvents;

#endif /* __SIRC_EVENT_H */
