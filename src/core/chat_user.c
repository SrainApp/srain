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
    self->chat = chat;
    self->srv_user = srv_user;
    srn_server_user_attach_chat_user(srv_user, self);

    return self;
}

void srn_chat_user_free(SrnChatUser *self){
    srn_server_user_detach_chat_user(self->srv_user, self);
    g_free(self);
}

void srn_chat_user_update(SrnChatUser *self){
    // TODO: rename nick
    sui_ren_user(self->chat->ui,
            self->srv_user->nick,
            self->srv_user->nick,
            self->type);
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
        sui_add_user(self->chat->ui, self->srv_user->nick, USER_CHIGUA);
    } else {
        sui_rm_user(self->chat->ui, self->srv_user->nick);
    }
}
