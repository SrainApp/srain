/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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

#ifndef __SRAIN_USER_LIST_H
#define __SRAIN_USER_LIST_H

#include <gtk/gtk.h>

#include "sui/sui.h"

#define SRAIN_TYPE_USER_LIST (srain_user_list_get_type())
#define SRAIN_USER_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_USER_LIST, SrainUserList))

typedef struct _SrainUserList SrainUserList;
typedef struct _SrainUserListClass SrainUserListClass;

GType srain_user_list_get_type(void);
SrainUserList *srain_user_list_new(void);
SrnRet srain_user_list_add(SrainUserList *list, const char *nick, UserType type);
SrnRet srain_user_list_rm(SrainUserList *list, const char *nick);
SrnRet srain_user_list_rename(SrainUserList *list, const char *old_nick,
                           const char *new_nick, UserType type);
void srain_user_list_clear(SrainUserList *list);

#endif /* __SRAIN_USER_LIST_H */
