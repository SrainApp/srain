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

#ifndef __SUI_JOIN_PANEL_H
#define __SUI_JOIN_PANEL_H

#include <gtk/gtk.h>

#define SUI_TYPE_JOIN_PANEL (sui_join_panel_get_type())
#define SUI_JOIN_PANEL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_JOIN_PANEL, SuiJoinPanel))
#define SUI_IS_JOIN_PANEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_JOIN_PANEL))

#define SUI_JOIN_PANEL_RESP_CANCEL   0
#define SUI_JOIN_PANEL_RESP_JOIN     1

typedef struct _SuiJoinPanel SuiJoinPanel;
typedef struct _SuiJoinPanelClass SuiJoinPanelClass;

GType sui_join_panel_get_type(void);
SuiJoinPanel* sui_join_panel_new();

void sui_join_panel_clear(SuiJoinPanel *self);

void sui_join_panel_set_model(SuiJoinPanel *self, GtkTreeModel *model);
void sui_join_panel_set_is_adding(SuiJoinPanel *self, bool is_adding);
bool sui_join_panel_get_is_adding(SuiJoinPanel *self);

#endif /* __SUI_JOIN_PANEL_H */
