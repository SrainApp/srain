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

#ifndef __SUI_WINDOW_H
#define __SUI_WINDOW_H

#include <gtk/gtk.h>

#include "sui_app.h"
#include "sui_buffer.h"

#define SUI_TYPE_WINDOW (sui_window_get_type())
#define SUI_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_WINDOW, SuiWindow))
#define SUI_IS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_WINDOW))

typedef struct _SuiWindow SuiWindow;
typedef struct _SuiWindowClass SuiWindowClass;

GType sui_window_get_type(void);
SuiWindow* sui_window_new(SuiApplication *app, SuiWindowEvents *events, SuiWindowConfig *cfg);

SuiWindowEvents* sui_window_get_events(SuiWindow *sui);
void sui_window_set_config(SuiWindow *self, SuiWindowConfig *cfg);
SuiWindowConfig* sui_window_get_config(SuiWindow *self);

void sui_window_add_buffer(SuiWindow *self, SuiBuffer *buffer);
void sui_window_rm_buffer(SuiWindow *self, SuiBuffer *buffer);
SuiBuffer *sui_window_get_buffer(SuiWindow *self, const char *name, const char *remark);
SuiBuffer *sui_window_get_cur_buffer(SuiWindow *self);
void sui_window_set_cur_buffer(SuiWindow *self, SuiBuffer *buf);
SuiSideBar* sui_window_get_side_bar(SuiWindow *self);
void sui_window_toggle_server_visibility(SuiWindow* self);

int sui_window_is_active(SuiWindow *self);
void sui_window_tray_icon_stress(SuiWindow *self, int stress);

#endif /* __SUI_WINDOW_H */
