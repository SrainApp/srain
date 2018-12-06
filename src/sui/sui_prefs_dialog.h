/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __SUI_PREFS_DIALOG_H
#define __SUI_PREFS_DIALOG_H

#include <gtk/gtk.h>

#include "sui_app.h"
#include "sui_window.h"

#define SUI_TYPE_PREFS_DIALOG (sui_prefs_dialog_get_type())
#define SUI_PREFS_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_PREFS_DIALOG, SuiPrefsDialog))
#define SUI_IS_PREFS_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_PREFS_DIALOG))

typedef struct _SuiPrefsDialog SuiPrefsDialog;
typedef struct _SuiPrefsDialogClass SuiPrefsDialogClass;

GType sui_prefs_dialog_get_type(void);
SuiPrefsDialog* sui_prefs_dialog_new(SuiApplication *app, SuiWindow *win);

#endif /* __SUI_PREFS_DIALOG_H */
