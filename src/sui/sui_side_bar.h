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

#ifndef __SUI_SIDE_BAR_H
#define __SUI_SIDE_BAR_H

#include <gtk/gtkbin.h>
#include <gtk/gtkstack.h>

#include "sui_buffer.h"

#define SUI_TYPE_SIDE_BAR           (sui_side_bar_get_type())
#define SUI_SIDE_BAR(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_SIDE_BAR, SuiSideBar))
#define SUI_IS_SIDE_BAR(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_SIDE_BAR))

typedef struct _SuiSideBar        SuiSideBar;
typedef struct _SuiSideBarClass   SuiSideBarClass;

GType sui_side_bar_get_type();
SuiSideBar* sui_side_bar_new();
void sui_side_bar_set_stack(SuiSideBar *self, GtkStack *stack);
GtkStack *sui_side_bar_get_stack(SuiSideBar *self);
void sui_side_bar_update(SuiSideBar *self, SuiBuffer *buffer, const char *nick, const char *msg, int is_visible);
void sui_side_bar_prev(SuiSideBar *self);
void sui_side_bar_next(SuiSideBar *self);

#endif /* __SUI_SIDE_BAR_H */
