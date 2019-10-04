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

#ifndef __SUI_USER_H
#define __SUI_USER_H

#include <gtk/gtk.h>

typedef struct _SuiUser SuiUser;
typedef struct _SuiUserStat SuiUserStat;

struct _SuiUserStat {
    int full_op;
    int half_op;
    int voiced;
    int total;
};

SuiUser *sui_user_new(void *ctx);
SuiUser *sui_user_new_from_iter(GtkListStore *list_store, GtkTreeIter *iter);
void sui_user_free(SuiUser *self);

void sui_user_update(SuiUser *self, GtkStyleContext *style_context);
int sui_user_compare(SuiUser *user1, SuiUser *user2);

void sui_user_set_list(SuiUser *self, GtkListStore *list);
void sui_user_set_stat(SuiUser *self, SuiUserStat *stat);
void* sui_user_get_ctx(SuiUser *self);
const char* sui_user_get_nickname(SuiUser *self);

#endif /* __SUI_USER_H */
