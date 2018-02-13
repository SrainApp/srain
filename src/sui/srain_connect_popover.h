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

#ifndef __SRAIN_CONNECT_POPOVER_H
#define __SRAIN_CONNECT_POPOVER_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_CONNECT_POPOVER (srain_connect_popover_get_type())
#define SRAIN_CONNECT_POPOVER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CONNECT_POPOVER, SrainConnectPopover))
#define SRAIN_IS_CONNECT_POPOVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CONNECT_POPOVER))

typedef struct _SrainConnectPopover SrainConnectPopover;
typedef struct _SrainConnectPopoverClass SrainConnectPopoverClass;

GType srain_connect_popover_get_type(void);
SrainConnectPopover* srain_connect_popover_new(GtkWidget *relative);
void srain_connect_popover_add_server(SrainConnectPopover *popover, const char *server);
void srain_connect_popover_clear(SrainConnectPopover *popover);

#endif /* __SRAIN_CONNECT_POPOVER_H */
