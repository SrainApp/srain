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

SrnMessage* srn_message_new(SrnChat *chat, SrnChatUser *user,
        const char *content, SrnMessageType type){
    SrnMessage *self;

    g_return_val_if_fail(chat, NULL);
    g_return_val_if_fail(user, NULL);
    g_return_val_if_fail(user->srv_user, NULL);

    self = g_malloc0(sizeof(SrnMessage));

    if (!content) {
        content = "";
    }

    self->user = user;
    self->chat = chat;
    self->content = g_strdup(content);
    self->time = g_date_time_new_now_local();
    self->mentioned = FALSE;
    self->type = type;

    /* Decorated */
    self->dname = g_markup_escape_text(user->srv_user->nick, -1);
    self->dcontent = g_markup_escape_text(content, -1);

    switch (self->type){
        case SRN_MESSAGE_TYPE_SENT:
            self->ui = sui_new_send_message(self);
            break;
        case SRN_MESSAGE_TYPE_RECV:
            self->ui = sui_new_recv_message(self);
            break;
        case SRN_MESSAGE_TYPE_MISC:
            self->ui = sui_new_misc_message(self, SUI_MISC_MESSAGE_STYLE_NORMAL);
            break;
        case SRN_MESSAGE_TYPE_ERROR:
            self->ui = sui_new_misc_message(self, SUI_MISC_MESSAGE_STYLE_ERROR);
            break;
        case SRN_MESSAGE_TYPE_ACTION:
            self->ui = sui_new_misc_message(self, SUI_MISC_MESSAGE_STYLE_ACTION);
            break;
        default:
            self->ui = sui_new_misc_message(self, SUI_MISC_MESSAGE_STYLE_NORMAL);
            g_warn_if_reached();
    }

    return self;
}

char* srn_message_to_string(SrnMessage *self){
    char *time_str;
    char *msg_str;

    time_str = g_date_time_format(self->time, "%T");
    g_return_val_if_fail(time_str, NULL);

    switch (self->type){
        case SRN_MESSAGE_TYPE_SENT:
            msg_str = g_strdup_printf("[%s] <%s*> %s",
                    time_str, self->user->srv_user->nick, self->content);
            break;
        case SRN_MESSAGE_TYPE_RECV:
        case SRN_MESSAGE_TYPE_NOTICE:
            msg_str = g_strdup_printf("[%s] <%s> %s",
                    time_str, self->user->srv_user->nick, self->content);
            break;
        case SRN_MESSAGE_TYPE_ACTION:
            msg_str = g_strdup_printf("[%s] * %s %s",
                    time_str, self->user->srv_user->nick, self->content);
            break;
        case SRN_MESSAGE_TYPE_MISC:
            msg_str = g_strdup_printf("[%s] = %s", time_str, self->content);
            break;
        case SRN_MESSAGE_TYPE_ERROR:
            msg_str = g_strdup_printf("[%s] ! %s", time_str, self->content);
            break;
        case SRN_MESSAGE_TYPE_UNKNOWN:
        default:
            g_warn_if_reached();
            msg_str = NULL;
            break;
    }

    g_free(time_str);

    return msg_str;
}

void srn_message_free(SrnMessage *self){
    str_assign(&self->dname, NULL);
    str_assign(&self->role, NULL);
    str_assign(&self->content, NULL);
    str_assign(&self->dcontent, NULL);
    g_date_time_unref(self->time);
    g_slist_free_full(self->urls, g_free);
    g_free(self);
}
