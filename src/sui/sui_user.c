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
#define COL_TYPE    3

/**
 * @brief SuiUser is a iterator of SuiUserList.
 */
struct _SuiUser {
    GtkTreeIter iter; // Can be used as a GtkTreeIter

    SrnChatUser *ctx;
    SrnChatUserType type;

    GtkListStore *list;
    SuiUserStat *stat;
};

// static cairo_surface_t* new_user_icon_from_type(SrnChatUserType type,
        // GtkStyleContext *style_context, GdkWindow *window);

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiUser *sui_user_new(void *ctx){
    SuiUser *self;

    self = g_malloc0(sizeof(SuiUser));
    self->ctx = ctx;
    self->type = self->ctx->type;

    return self;
}

SuiUser *sui_user_new_from_iter(GtkListStore *list, GtkTreeIter *iter){
    SuiUser *self;

    self = g_malloc0(sizeof(SuiUser));
    self->list = list;
    self->iter = *iter; // Copy iter
    gtk_tree_model_get(GTK_TREE_MODEL(list), iter,
            COL_USER, &self->ctx,
            COL_TYPE, &self->type,
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

void sui_user_update(SuiUser *self, GtkStyleContext *style_context,
        void *window){
    g_return_if_fail(self->list);
    g_return_if_fail(self->stat);
    g_return_if_fail(self->ctx);

    // Update stat
    if (self->ctx->type != self->type){
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
        switch (self->type) {
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
    self->type = self->ctx->type;

    // cairo_surface_t *icon = new_user_icon_from_type(self->ctx->type,
            // style_context, window);
    gtk_list_store_set(self->list, (GtkTreeIter *)self,
            COL_NAME, self->ctx->srv_user->nick,
            // COL_ICON, icon,
            COL_USER, self->ctx,
            COL_TYPE, self->ctx->type,
            -1);
    // cairo_surface_destroy(icon);
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

// static cairo_surface_t* new_user_icon_from_type(SrnChatUserType type,
//         GtkStyleContext *style_context, GdkWindow *window){
//     const char *color_str;
//     GError *err;
//     GdkRGBA fg_color;
//     GdkPixbuf *pixbuf;
//     GtkIconInfo *icon_info;
//     cairo_surface_t *surface;
// 
//     switch (type){
//         case SRN_CHAT_USER_TYPE_ADMIN:
//         case SRN_CHAT_USER_TYPE_OWNER:
//         case SRN_CHAT_USER_TYPE_FULL_OP:
//             color_str = "#157915";
//             break;
//         case SRN_CHAT_USER_TYPE_HALF_OP:
//             color_str = "#856117";
//             break;
//         case SRN_CHAT_USER_TYPE_VOICED:
//             color_str = "#451984";
//             break;
//         case SRN_CHAT_USER_TYPE_CHIGUA:
//             color_str = NULL;
//             break;
//         default:
//             color_str = NULL;
//             g_warn_if_reached();
//     }
//     if (color_str && !gdk_rgba_parse(&fg_color, color_str)) {
//         ERR_FR("Failed to parser color str %s", color_str);
//     }
// 
//     icon_info = gtk_icon_theme_lookup_icon_for_scale(
//             gtk_icon_theme_get_default(),
//             "user-available",
//             16,
//             gdk_window_get_scale_factor(window),
//             GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
//     if (!icon_info) {
//         icon_info = gtk_icon_theme_lookup_icon_for_scale(
//                 gtk_icon_theme_get_default(),
//                 "user-available",
//                 16,
//                 gdk_window_get_scale_factor(window),
//                 0);
//     }
//     g_return_val_if_fail(icon_info, NULL);
// 
//     err = NULL;
//     if (color_str) {
//         pixbuf = gtk_icon_info_load_symbolic(icon_info, &fg_color,
//                 NULL, NULL, NULL, NULL, &err);
//     } else {
//         // Use default foreground color
//         pixbuf = gtk_icon_info_load_symbolic_for_context(icon_info,
//                 style_context, NULL, &err);
//     }
//     g_object_unref(icon_info);
//     if (err) {
//         ERR_FR("Failed to load user icon: %s", err->message);
//         g_error_free(err);
//     }
// 
//     g_return_val_if_fail(pixbuf, NULL);
//     surface = gdk_cairo_surface_create_from_pixbuf(pixbuf,
//             gdk_window_get_scale_factor(window), window);
//     g_object_unref(pixbuf);
//     return surface;
// }
