/*
 * Copyright (C) 2021 Val Lorentz <progval+srain@progval.net>
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

#include "sirc/sirc.h"

struct _SircMessageContext {
    GDateTime *time; 
};

SircMessageContext* sirc_message_context_new(GDateTime* time) {
    SircMessageContext *context;

    if (!time) {
        time = g_date_time_new_now_local();
    }

    context = g_malloc0(sizeof(SircMessageContext));
    context->time = time;

    return context;
}

GDateTime* sirc_message_context_get_time(SircMessageContext *context) {
    g_return_val_if_fail(context, NULL);
    return context->time;
}

void sirc_message_context_free(SircMessageContext *context) {
    g_return_if_fail(context);
    g_date_time_unref(context->time);
    g_free(context);
}
