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

Message* message_new(SrnChat *chat, SrnUser *user, const char *content, MessageType type){
    Message *msg;

    g_return_val_if_fail(chat, NULL);
    g_return_val_if_fail(user, NULL);
    g_return_val_if_fail(content, NULL);

    msg = g_malloc0(sizeof(Message));

    msg->user = srn_user_ref(user);
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

void message_free(Message *msg){
    if (msg->chat) { /* Nothing to do. */ }
    if (msg->ui) { /* Nothing to do. */ }

    if (msg->user) {
        srn_user_free(msg->user);
    }

    if (msg->urls) {
        GSList *lst = msg->urls;
        while (lst){
            g_free(lst->data);
            lst->data = NULL;
            lst = g_slist_next(lst);
        }
        g_slist_free(msg->urls);
    }

    if (msg->dname) {
        g_free(msg->dname);
    }

    if (msg->role) {
        g_free(msg->role);
    }

    if (msg->content) {
        g_free(msg->content);
    }

    if (msg->dcontent) {
        g_free(msg->dcontent);
    }

    g_free(msg);
}
