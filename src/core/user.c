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


#include "core/core.h"

#include "log.h"
#include "utils.h"

typedef struct _SrnUserContext SrnUserContext;

struct _SrnUserContext {
    SrnChat *chat;
    SrnUserType type;
};

SrnUser *srn_user_new(SrnServer *srv, const char *nick){
    SrnUser *user;

    g_return_val_if_fail(srv, NULL);
    g_return_val_if_fail(nick, NULL);

    user = g_malloc0(sizeof(SrnUser));
    user->srv = srv;
    user->me = FALSE;
    str_assign(&user->nick, nick);

    return user;
}

void srn_user_free(SrnUser *user){
    g_return_if_fail(g_slist_length(user->chat_list) == 0);

    str_assign(&user->nick, NULL);
    str_assign(&user->username, NULL);
    str_assign(&user->hostname, NULL);
    str_assign(&user->realname, NULL);
    g_free(user);
}

SrnRet srn_user_attach_chat(SrnUser *user, SrnChat *chat, SrnUserType type){
    GSList *lst;
    SrnUserContext *ctx;

    lst = user->chat_list;
    while (lst){
        ctx = lst->data;
        if (ctx->chat == chat){
            return SRN_ERR;
        }
    }

    ctx = g_malloc0(sizeof(SrnUserContext));
    ctx->chat = chat;
    ctx->type = type;
    sui_add_user(chat->ui, user->nick, type);

    return SRN_OK;
}

SrnRet srn_user_detach_chat(SrnUser *user, SrnChat *chat){
    GSList *lst;
    SrnUserContext *ctx;

    lst = user->chat_list;
    while (lst){
        ctx = lst->data;
        if (ctx->chat == chat){
            user->chat_list = g_slist_delete_link(user->chat_list, lst);
            sui_rm_user(chat->ui, user->nick);
            g_free(ctx);
            return SRN_OK;
        }
    }

    return SRN_ERR;
}

void srn_user_set_nick(SrnUser *user, const char *nick){
    GSList *lst;

    str_assign(&user->nick, nick);

    /* Update UI status */
    lst = user->chat_list;
    while (lst) {
        SrnUserContext *ctx;

        ctx = lst->data;
        sui_ren_user(ctx->chat->ui, user->nick, nick, ctx->type);
    }
}

void srn_user_set_username(SrnUser *user, const char *username){
    str_assign(&user->username, username);
}

void srn_user_set_hostname(SrnUser *user, const char *hostname){
    str_assign(&user->hostname, hostname);
}

void srn_user_set_realname(SrnUser *user, const char *realname){
    str_assign(&user->realname, realname);
}

void srn_user_set_me(SrnUser *user, bool me){
    user->me = me;
}

void srn_user_set_type(SrnUser *user, SrnChat *chat, SrnUserType type){
    GSList *lst;

    /* Update UI status */
    lst = user->chat_list;
    while (lst) {
        SrnUserContext *ctx;

        ctx = lst->data;
        if (ctx->chat == chat){
            ctx->type = type;
            sui_ren_user(ctx->chat->ui, user->nick, user->nick, type);
            return;
        }
    }
}

