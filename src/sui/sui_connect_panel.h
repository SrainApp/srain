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

#ifndef __SUI_CONNECT_PANEL_H
#define __SUI_CONNECT_PANEL_H

#include <gtk/gtk.h>

#define SUI_TYPE_CONNECT_PANEL (sui_connect_panel_get_type())
#define SUI_CONNECT_PANEL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_CONNECT_PANEL, SuiConnectPanel))
#define SUI_IS_CONNECT_PANEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_CONNECT_PANEL))

typedef struct _SuiConnectPanel SuiConnectPanel;
typedef struct _SuiConnectPanelClass SuiConnectPanelClass;

GType sui_connect_panel_get_type(void);
SuiConnectPanel* sui_connect_panel_new();
void sui_connect_panel_add_server(SuiConnectPanel *popover, const char *server);
void sui_connect_panel_clear(SuiConnectPanel *popover);

#endif /* __SUI_CONNECT_PANEL_H */
