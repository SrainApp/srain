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

#ifndef __SUI_USER_LIST_H
#define __SUI_USER_LIST_H

#include <gtk/gtk.h>

#include "sui_user.h"

typedef struct _SuiUserList SuiUserList;
typedef struct _SuiUserListClass SuiUserListClass;

#define SUI_TYPE_USER_LIST (sui_user_list_get_type())
#define SUI_USER_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_USER_LIST, SuiUserList))

GType sui_user_list_get_type(void);
SuiUserList *sui_user_list_new(void);

void sui_user_list_add_user(SuiUserList *list, SuiUser *user);
void sui_user_list_rm_user(SuiUserList *list, SuiUser *user);
void sui_user_list_update_user(SuiUserList *list, SuiUser *user);
void sui_user_list_clear(SuiUserList *list);
GList* sui_user_list_get_users_by_prefix(SuiUserList *self, const char *prefix);

#endif /* __SUI_USER_LIST_H */
