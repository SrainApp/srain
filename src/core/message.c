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

#include <glib.h>

#include "core/core.h"

#include "srain.h"
#include "utils.h"

SrnMessage* srn_message_new(SrnChat *chat, SrnUser *user, const char *content, SrnMessageType type){
    SrnMessage *msg;

    g_return_val_if_fail(chat, NULL);
    g_return_val_if_fail(user, NULL);
    g_return_val_if_fail(content, NULL);

    msg = g_malloc0(sizeof(SrnMessage));

    msg->user = user;
    msg->chat = chat;
    // msg->role = NULL; // via g_malloc0()
    msg->content = g_strdup(content);
    msg->time = get_current_time_s();
    msg->mentioned = FALSE;
    msg->type = type;
    // msg->urls = NULL; // via g_malloc0()
    // msg->ui = NULL; // via g_malloc0()

    /* Decorated */
    msg->dname = g_strdup(user->nick);
    msg->dcontent = g_markup_escape_text(content, -1);

    return msg;
}

void srn_message_free(SrnMessage *msg){
    str_assign(&msg->dname, NULL);
    str_assign(&msg->role, NULL);
    str_assign(&msg->content, NULL);
    str_assign(&msg->dcontent, NULL);
    g_slist_free_full(msg->urls, g_free);
    g_free(msg);
}
