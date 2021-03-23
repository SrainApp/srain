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

#include <gtk/gtk.h>

#include "sif/sif-messenger.h"

/*********************
 * GObject functions *
 *********************/

G_DEFINE_INTERFACE(SifMessenger, sif_messenger, G_TYPE_OBJECT)

static void
sif_messenger_default_init(SifMessengerInterface *iface) {
    /**
     * SifMessenger:schemas
     *
     * Semicolen secparated URL schemas handled by of #SifMessenger.
     */
    g_object_interface_install_property(iface,
                                        g_param_spec_string("schemas",
                                                "Schemas",
                                                "URL Schemas of SifMessenger",
                                                NULL,
                                                G_PARAM_READABLE));
}
