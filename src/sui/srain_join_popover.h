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

#ifndef __SRAIN_JOIN_POPOVER_H
#define __SRAIN_JOIN_POPOVER_H

#include <gtk/gtk.h>
#include "srain_server_buffer.h"

#define SRAIN_TYPE_JOIN_POPOVER (srain_join_popover_get_type())
#define SRAIN_JOIN_POPOVER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_JOIN_POPOVER, SrainJoinPopover))
#define SRAIN_IS_JOIN_POPOVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_JOIN_POPOVER))

#define SRAIN_JOIN_POPOVER_RESP_CANCEL   0
#define SRAIN_JOIN_POPOVER_RESP_JOIN     1

typedef struct _SrainJoinPopover SrainJoinPopover;
typedef struct _SrainJoinPopoverClass SrainJoinPopoverClass;

GType srain_join_popover_get_type(void);
SrainJoinPopover* srain_join_popover_new(GtkWidget *relative);

void srain_join_popover_prepare_model(SrainJoinPopover *popover, SrainServerBuffer *buf);
void srain_join_popover_set_model(SrainJoinPopover *popover, SrainServerBuffer *buf);

void srain_join_popover_start_chan(SrainJoinPopover *dialog);
void srain_join_popover_add_chan(SrainJoinPopover *dialog, const char *chan,
        int users, const char *topic);
void srain_join_popover_end_chan(SrainJoinPopover *dialog);
void srain_join_popover_clear(SrainJoinPopover *popover);

#endif /* __SRAIN_JOIN_POPOVER_H */
