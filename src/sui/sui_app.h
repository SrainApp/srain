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

#ifndef __SUI_APP_H
#define __SUI_APP_H

#include <gtk/gtk.h>

#include "sui/sui.h"

#define SUI_TYPE_APPLICATION (sui_application_get_type())
#define SUI_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_APPLICATION, SuiApplication))

typedef struct _SuiApplication SuiApplication;
typedef struct _SuiApplicationClass SuiApplicationClass;

GType sui_application_get_type(void);
SuiApplication* sui_application_new(const char *id, SuiApplicationEvents *events, SuiApplicationConfig *cfg);
void sui_application_run(SuiApplication *self, int argc, char *argv[]);
void sui_application_quit(SuiApplication *self);

SuiWindow* sui_application_get_cur_window(SuiApplication *self);
SuiApplication* sui_application_get_instance();
SuiApplicationEvents* sui_application_get_events(SuiApplication *self);
void sui_application_set_config(SuiApplication *self, SuiApplicationConfig *cfg);
void* sui_application_get_ctx(SuiApplication *self);
void sui_application_set_ctx(SuiApplication *self, void *ctx);

#endif /* __SUI_APP_H */
