/*
 * Copyright (C) 2014 Intel Corporation
 * Copyright (C) 2016-2017 Shengyu Zhang <silverrain@outlook.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author:
 *      Ikey Doherty <michael.i.doherty@intel.com>
 */

#ifndef __SRAIN_STACK_SIDEBAR_H__
#define __SRAIN_STACK_SIDEBAR_H__

#include <gtk/gtkbin.h>
#include <gtk/gtkstack.h>
#include "srain_chat.h"

#define SRAIN_TYPE_STACK_SIDEBAR           (srain_stack_sidebar_get_type())
#define SRAIN_STACK_SIDEBAR(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_STACK_SIDEBAR, SrainStackSidebar))
#define SRAIN_IS_STACK_SIDEBAR(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_STACK_SIDEBAR))

typedef struct _SrainStackSidebar        SrainStackSidebar;
typedef struct _SrainStackSidebarClass   SrainStackSidebarClass;

GType srain_stack_sidebar_get_type();
SrainStackSidebar* srain_stack_sidebar_new();
void srain_stack_sidebar_set_stack(SrainStackSidebar *sidebar, GtkStack *stack);
GtkStack *srain_stack_sidebar_get_stack(SrainStackSidebar *sidebar);
void srain_stack_sidebar_update(SrainStackSidebar *sidebar, SrainChat *chat, const char *nick, const char *msg, int is_visible);
void srain_stack_sidebar_prev(SrainStackSidebar *sidebar);
void srain_stack_sidebar_next(SrainStackSidebar *sidebar);

#endif /* __SRAIN_STACK_SIDEBAR_H__ */
