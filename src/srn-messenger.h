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

#ifndef __SNR_APP_H
#define __SNR_APP_H

#include <gtk/gtk.h>

#define SRN_TYPE_MESSENGER (srn_messenger_get_type())
#define SRN_MESSENGER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRN_TYPE_MESSENGER, SrnMessenger))
#define SRN_IS_MESSENGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRN_TYPE_MESSENGER))

typedef struct _SrnMessenger SrnMessenger;
typedef struct _SrnMessengerClass SrnMessengerClass;

GType srn_messenger_get_type(void);

#endif /* __SRN_APP_H */
