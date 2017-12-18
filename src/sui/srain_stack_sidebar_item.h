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

#ifndef __SRAIN_STACK_SIDEBAR_ITEM_H
#define __SRAIN_STACK_SIDEBAR_ITEM_H

#include <gtk/gtk.h>

#include "srain_buffer.h"

#define SRAIN_TYPE_STACK_SIDEBAR_ITEM (srain_stack_sidebar_item_get_type())
#define SRAIN_STACK_SIDEBAR_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_STACK_SIDEBAR_ITEM, SrainStackSidebarItem))
#define SRAIN_IS_STACK_SIDEBAR_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_STACK_SIDEBAR_ITEM))

typedef struct _SrainStackSidebarItem SrainStackSidebarItem;
typedef struct _SrainStackSidebarItemClass SrainStackSidebarItemClass;

GType srain_stack_sidebar_item_get_type(void);
SrainStackSidebarItem *srain_stack_sidebar_item_new(const char *name, const char *remark, const char *icon);
void srain_stack_sidebar_item_count_clear(SrainStackSidebarItem *item);
void srain_stack_sidebar_item_count_inc(SrainStackSidebarItem *item);
void srain_stack_sidebar_item_recentmsg_update(SrainStackSidebarItem *item, const char *nick, const char *msg);
unsigned long srain_stack_sidebar_item_get_update_time(SrainStackSidebarItem *item);

#endif /* __SRAIN_STACK_SIDEBAR_ITEM_H */

