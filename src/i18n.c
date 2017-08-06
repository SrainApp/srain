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

#include "i18n.h"
#include "log.h"

void i18n_init(){
    // Set the current local to default
    setlocale (LC_ALL, "");

    /* Specify that the DOMAINNAME message catalog
     * will be found in DIRNAME rather than in
     * the system locale data base
     */
    bindtextdomain (GETTEXT_PACKAGE,  PACKAGE_DATA_DIR "/share/locale");
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

    // Set the current default message catalog to DOMAINNAME.
    textdomain (GETTEXT_PACKAGE);

    LOG_FR(_("Language: English"));
}
