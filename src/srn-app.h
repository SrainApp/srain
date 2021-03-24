/* Copyright (C) 2016-2021 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __SRN_APP_H
#define __SRN_APP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SRN_TYPE_APPLICATION srn_application_get_type()
G_DECLARE_FINAL_TYPE(SrnApplication, srn_application, SRN, APPLICATION,
                     GtkApplication)

void srn_application_ping(SrnApplication *self);
SrnApplication *srn_application_get_instance();

G_END_DECLS

#endif /* __SRN_APP_H */
