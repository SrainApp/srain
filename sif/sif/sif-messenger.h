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

#ifndef __SIF_MESSENGER_H
#define __SIF_MESSENGER_H

#include <glib-object.h>

#include "sif-flow.h"

G_BEGIN_DECLS

#define SIF_TYPE_MESSENGER (sif_messenger_get_type ())

G_DECLARE_INTERFACE(SifMessenger, sif_messenger, SIF, MESSENGER, GObject)

struct _SifMessengerInterface {
    GTypeInterface parent_iface;

    SifFlow *(*login)(SifMessenger *self);
    SifFlow *(*contact)(SifMessenger *self);
};

G_END_DECLS

#endif /* __SIF_MESSENGER_H */
