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

#ifndef __SRN_MESSENGER_H
#define __SRN_MESSENGER_H

#include <glib-object.h>

#include "srn-flow.h"

G_BEGIN_DECLS

#define SRN_TYPE_MESSENGER (srn_messenger_get_type ())

G_DECLARE_INTERFACE(SrnMessenger, srn_messenger, SRN, MESSENGER, GObject)

struct _SrnMessengerInterface {
    GTypeInterface parent_iface;

    SrnFlow *(*login)(SrnMessenger *self);
    SrnFlow *(*contact)(SrnMessenger *self);
};

G_END_DECLS

#endif /* __SRN_MESSENGER_H */
