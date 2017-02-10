/*
 * Copyright (C) 2004-2012 George Yunaev <gyunaev@ulduzsoft.com>
 *               2017      Shengyu Zhang <silverainz@outlook.com>
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

/* This file is borrowed from libircclient(include/libirc_events.h) */

#ifndef __SIRC_EVENT_H
#define __SIRC_EVENT_H

#include "sirc.h"
#include "sirc_parse.h"

void sirc_event_hdr(SircSession *sirc, SircMessage *imsg);

#endif /* __SIRC_EVENT_H */
