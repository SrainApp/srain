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

#ifndef __SRAIN_WINDOW_H
#define __SRAIN_WINDOW_H

#include <gtk/gtk.h>
#include "srain_app.h"
#include "srain_buffer.h"
#include "srain_server_buffer.h"
#include "srain_connect_popover.h"
#include "srain_join_popover.h"

#define SRAIN_TYPE_WINDOW (srain_window_get_type())
#define SRAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_WINDOW, SrainWindow))
#define SRAIN_IS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_WINDOW))

typedef struct _SrainWindow SrainWindow;
typedef struct _SrainWindowClass SrainWindowClass;

GType srain_window_get_type(void);
SrainWindow *srain_window_new(SrainApp *app);

void srain_window_add_buffer(SrainWindow *win, SrainBuffer *buffer);
void srain_window_rm_buffer(SrainWindow *win, SrainBuffer *buffer);
SrainBuffer *srain_window_get_buffer(SrainWindow *win, const char *name, const char *remark);
SrainBuffer *srain_window_get_cur_buffer(SrainWindow *win);
SrainServerBuffer* srain_window_get_cur_server_buffer(SrainWindow *win);

void srain_window_spinner_toggle(SrainWindow *win, gboolean is_busy);
void srain_window_stack_sidebar_update(SrainWindow *win, SrainBuffer *buffer, const char *nick, const char *msg);
int srain_window_is_active(SrainWindow *win);
void srain_window_tray_icon_stress(SrainWindow *win, int stress);

SrainConnectPopover *srain_window_get_connect_popover(SrainWindow *win);
SrainJoinPopover *srain_window_get_join_popover(SrainWindow *win);

/* Only one SrainWindow instance in one application */
extern SrainWindow *srain_win;

#endif /* __SRAIN_WINDOW_H */
