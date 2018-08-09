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

/**
  @file sui_user.c
 * @brief
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 
 * @date 2018-06-03
 */

#include <gtk/gtk.h>

#include "core/core.h"

#include "sui_user.h"

#define COL_NAME    0
#define COL_ICON    1
#define COL_USER    2

/**
 * @brief SuiUser is a iterator of SuiUserList.
 */
struct _SuiUser {
    GtkTreeIter iter; // Can be used as a GtkTreeIter

    SrnChatUser *ctx;

    GtkListStore *list;
    SuiUserStat *stat;
};

static const char* user_type_to_icon_name(SrnChatUserType type);
static SrnChatUserType icon_name_to_user_type(const char *icon);

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiUser *sui_user_new(void *ctx){
    SuiUser *self;

    self = g_malloc0(sizeof(SuiUser));
    self->ctx = ctx;

    return self;
}

SuiUser *sui_user_new_from_iter(GtkListStore *list, GtkTreeIter *iter){
    SuiUser *self;

    self = sui_user_new(NULL);
    self->list = list;
    self->iter = *iter; // Copy iter
    gtk_tree_model_get(GTK_TREE_MODEL(list), iter,
            COL_USER, &self->ctx,
            -1);

    return self;
}

void sui_user_free(SuiUser *self){
    g_free(self);
}

int sui_user_compare(SuiUser *user1, SuiUser *user2){
    g_return_val_if_fail(user1->ctx, 0);
    g_return_val_if_fail(user2->ctx, 0);

    if (user1->ctx->type != user2->ctx->type){
        return user1->ctx->type - user2->ctx->type;
    }
    return g_ascii_strcasecmp(
            user1->ctx->srv_user->nick,
            user2->ctx->srv_user->nick);
}

void sui_user_update(SuiUser *self){
    char *icon;
    SrnChatUserType type;

    g_return_if_fail(self->list);
    g_return_if_fail(self->stat);
    g_return_if_fail(self->ctx);

    // Update stat
    gtk_tree_model_get(GTK_TREE_MODEL(self->list), (GtkTreeIter *)self,
            COL_ICON, &icon,
            -1);
    type = icon_name_to_user_type(icon);
    g_free(icon);
    if (self->ctx->type != type){
        switch (self->ctx->type) {
        case SRN_CHAT_USER_TYPE_ADMIN:
        case SRN_CHAT_USER_TYPE_OWNER:
        case SRN_CHAT_USER_TYPE_FULL_OP:
            self->stat->full_op++;
            break;
        case SRN_CHAT_USER_TYPE_HALF_OP:
            self->stat->half_op++;
            break;
        case SRN_CHAT_USER_TYPE_VOICED:
            self->stat->voiced++;
            break;
        default:
            ;
        }
        switch (self->ctx->type) {
        case SRN_CHAT_USER_TYPE_ADMIN:
        case SRN_CHAT_USER_TYPE_OWNER:
        case SRN_CHAT_USER_TYPE_FULL_OP:
            self->stat->full_op--;
            break;
        case SRN_CHAT_USER_TYPE_HALF_OP:
            self->stat->half_op--;
            break;
        case SRN_CHAT_USER_TYPE_VOICED:
            self->stat->voiced--;
            break;
        default:
            ;
        }
    }

    gtk_list_store_set(self->list, (GtkTreeIter *)self,
            COL_NAME, self->ctx->srv_user->nick,
            COL_ICON, user_type_to_icon_name(self->ctx->type),
            COL_USER, self->ctx,
            -1);
}

void sui_user_set_list(SuiUser *self, GtkListStore *list){
    // One of theme is NULL
    g_return_if_fail((self->list == NULL) ^ (list == NULL));
    self->list = list;
}

void sui_user_set_stat(SuiUser *self, SuiUserStat *stat){
    // One of theme is NULL
    g_return_if_fail((self->stat == NULL) ^ (stat == NULL));
    self->stat = stat;
}

void* sui_user_get_ctx(SuiUser *self){
    return self->ctx;
}

const char* sui_user_get_nickname(SuiUser *self){
    return self->ctx->srv_user->nick;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static const char *user_type_to_icon_name(SrnChatUserType type){
    const char *icon;

    switch (type){
        case SRN_CHAT_USER_TYPE_ADMIN:
        case SRN_CHAT_USER_TYPE_OWNER:
        case SRN_CHAT_USER_TYPE_FULL_OP:
            icon = "srain-user-full-op";
            break;
        case SRN_CHAT_USER_TYPE_HALF_OP:
            icon = "srain-user-half-op";
            break;
        case SRN_CHAT_USER_TYPE_VOICED:
            icon = "srain-user-voiced";
            break;
        case SRN_CHAT_USER_TYPE_CHIGUA:
            icon = "srain-person";
            break;
        default:
            g_warn_if_reached();
            icon = NULL;
    }

    return icon;
}

static SrnChatUserType icon_name_to_user_type(const char *icon){
    SrnChatUserType type;

    if (!icon || g_ascii_strcasecmp(icon, "srain-person") == 0){
        type = SRN_CHAT_USER_TYPE_CHIGUA;
    } else if (g_ascii_strcasecmp(icon, "srain-user-full-op") == 0){
        type = SRN_CHAT_USER_TYPE_FULL_OP;
    } else if (g_ascii_strcasecmp(icon, "srain-user-half-op") == 0){
        type = SRN_CHAT_USER_TYPE_HALF_OP;
    } else if (g_ascii_strcasecmp(icon, "srain-user-voiced") == 0){
        type = SRN_CHAT_USER_TYPE_VOICED;
    } else {
        g_warn_if_reached();
        type = SRN_CHAT_USER_TYPE_CHIGUA;
    }

    return type;
}
