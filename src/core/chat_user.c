/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
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

#include "core/core.h"

#include "log.h"
#include "utils.h"

SrnChatUser *srn_chat_user_new(SrnChat *chat, SrnServerUser *srv_user){
    SrnChatUser *self;

    self = g_malloc0(sizeof(SrnChatUser));
    self->type = SRN_CHAT_USER_TYPE_CHIGUA;
    self->chat = chat;
    self->srv_user = srv_user;
    self->is_ignored = FALSE;
    srn_server_user_attach_chat_user(srv_user, self);
    self->ui = sui_new_user(self);
    self->extra_data = srn_extra_data_new();

    return self;
}

void srn_chat_user_free(SrnChatUser *self){
    sui_free_user(self->ui);
    srn_server_user_detach_chat_user(self->srv_user, self);
    srn_extra_data_free(self->extra_data);
    g_free(self);
}

void srn_chat_user_update(SrnChatUser *self){
    if (self->is_joined) {
        sui_update_user(self->chat->ui, self->ui);
    }
}

void srn_chat_user_set_type(SrnChatUser *self, SrnChatUserType type){
    if (self->type == type){
        return;
    }
    self->type = type;
    srn_chat_user_update(self);
}

void srn_chat_user_set_is_joined(SrnChatUser *self, bool joined){
    if (self->is_joined == joined){
        return;
    }
    self->is_joined = joined;
    if (joined){
        sui_add_user(self->chat->ui, self->ui);
    } else {
        sui_rm_user(self->chat->ui, self->ui);
    }
}

void srn_chat_user_set_is_ignored(SrnChatUser *self, bool ignored){
    if (self->is_ignored == ignored){
        return;
    }
    self->is_ignored = ignored;
}
