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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "srn-extension.h"

G_DEFINE_INTERFACE(SrnExtension, srn_extension, G_TYPE_OBJECT)

static void
srn_extension_default_init(SrnExtensionInterface *iface) {
    /**
     * SrnExtension:name
     */
    g_object_interface_install_property(iface,
                                        g_param_spec_string("name",
                                                N_("Name"),
                                                N_("Name of extension"),
                                                NULL,
                                                G_PARAM_READABLE));
    /**
     * SrnExtension:pretty-name
     */
    g_object_interface_install_property(iface,
                                        g_param_spec_string("pretty-name",
                                                N_("Pretty Name"),
                                                N_("Titlecased pretty name of extension"),
                                                NULL,
                                                G_PARAM_READABLE));
    /**
     * SrnExtension:version
     */
    g_object_interface_install_property(iface,
                                        g_param_spec_int("version",
                                                N_("Version"),
                                                N_("Version of extension"),
                                                0, G_MAXINT, 0,
                                                G_PARAM_READABLE));
}
