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

#include <stdlib.h>
#include <signal.h>
#include <libintl.h>

// For i18n
#include <locale.h>
#include <glib/gi18n.h>

// For package meta infos
#include "srn-meta.h"
// For SrnApplication
#include "srn-app.h"

extern char *irdump;

int
main(int argc, char *argv[]) {
    // Set the current local to default
    setlocale(LC_ALL, "");
    // Specify that the DOMAINNAME message catalog
    // will be found in DIRNAME rather than in
    // the system locale data base
    bindtextdomain(GETTEXT_PACKAGE,  PACKAGE_DATA_DIR "/locale");
    bind_textdomain_codeset(GETTEXT_PACKAGE, PACKAGE_CODESET);
    // Set the current default message catalog to DOMAINNAME.
    textdomain(GETTEXT_PACKAGE);

    SrnApplication *app = srn_application_new();

    return g_application_run(G_APPLICATION(app), argc, argv);
}
