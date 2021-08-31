/*
 * Copyright (C) 2021 Val Lorentz <progval+srain@progval.net>
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

#ifndef __SIRC_CONTEXT_H
#define __SIRC_CONTEXT_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include <glib.h>

typedef struct _SircMessageContext SircMessageContext;

/*
 * @param time The original timestamp of the message. Defaults to now if NULL.
 */
SircMessageContext* sirc_message_context_new(GDateTime* time);
void sirc_message_context_free(SircMessageContext *context);

/* Server-provided "time" tag if any, or the time the message was received/sent.
 * Never returns NULL. */
const GDateTime* sirc_message_context_get_time(const SircMessageContext *context);


#endif /* __SIRC_CONTEXT_H */
