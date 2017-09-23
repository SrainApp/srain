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

#ifndef __SRAIN_JOIN_DIALOG_H
#define __SRAIN_JOIN_DIALOG_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_JOIN_DIALOG (srain_join_dialog_get_type())
#define SRAIN_JOIN_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_JOIN_DIALOG, SrainJoinDialog))
#define SRAIN_IS_JOIN_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_JOIN_DIALOG))

#define SRAIN_JOIN_DIALOG_RESP_CANCEL   0
#define SRAIN_JOIN_DIALOG_RESP_JOIN     1

typedef struct _SrainJoinDialog SrainJoinDialog;
typedef struct _SrainJoinDialogClass SrainJoinDialogClass;

GType srain_join_dialog_get_type(void);
SrainJoinDialog* srain_join_dialog_new(GtkWindow *parent);

#endif /* __SRAIN_JOIN_DIALOG_H */
