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

#include <glib.h>

#include "sirc/sirc.h"
#include "i18n.h"

SircPrefs *sirc_prefs_new(){
    SircPrefs *prefs;

    prefs = g_malloc0(sizeof(SircPrefs));

    return prefs;
}

SrnRet sirc_prefs_check(SircPrefs *prefs){
    if (!prefs){
        return RET_ERR(_("Invalid ServerPrefs instance"));
    }
    return SRN_OK;
}

char* sirc_prefs_dump(SircPrefs *prefs){
    GString *str;
    g_return_val_if_fail(prefs, NULL);

    const char *t = _("True");
    const char *f = _("False");

    str = g_string_new("");
    g_string_append_printf(str,
            _("TLS: %s, TLS verify certificate: %s"),
            prefs->tls ? t : f, prefs->tls_noverify ? f : t);

    char *dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

void sirc_prefs_free(SircPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
