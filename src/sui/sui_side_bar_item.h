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

#ifndef __SUI_SIDE_BAR_ITEM_H
#define __SUI_SIDE_BAR_ITEM_H

#include <gtk/gtk.h>

#define SUI_TYPE_SIDE_BAR_ITEM (sui_side_bar_item_get_type())
#define SUI_SIDE_BAR_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_SIDE_BAR_ITEM, SuiSideBarItem))
#define SUI_IS_SIDE_BAR_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_SIDE_BAR_ITEM))

typedef struct _SuiSideBarItem SuiSideBarItem;
typedef struct _SuiSideBarItemClass SuiSideBarItemClass;

GType sui_side_bar_item_get_type(void);
SuiSideBarItem *sui_side_bar_item_new(const char *name, const char *remark, const char *icon);

void sui_side_bar_item_update(SuiSideBarItem *self, const char *nick, const char *msg);
void sui_side_bar_item_highlight(SuiSideBarItem *self);
void sui_side_bar_item_inc_count(SuiSideBarItem *self);
void sui_side_bar_item_clear_count(SuiSideBarItem *self);

unsigned long sui_side_bar_item_get_update_time(SuiSideBarItem *self);

#endif /* __SUI_SIDE_BAR_ITEM_H */

