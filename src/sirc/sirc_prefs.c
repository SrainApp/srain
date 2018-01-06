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
#include "utils.h"

SircPrefs *sirc_prefs_new(){
    SircPrefs *prefs;

    prefs = g_malloc0(sizeof(SircPrefs));

    return prefs;
}

SrnRet sirc_prefs_check(SircPrefs *prefs){
    // const char *fmt = _("Missing field in SircPrefs: %1$s");

    if (!prefs){
        return RET_ERR(_("Invalid ServerPrefs instance"));
    }

    if (str_is_empty(prefs->encoding)) {
        str_assign(&prefs->encoding, "UTF-8");
    }

    /* Check encoding */
    {
        char *test;
        GError *err = NULL;

        test = g_convert("", -1,
                SRN_ENCODING, prefs->encoding,
                NULL, NULL, &err);
        if (err){
            return RET_ERR(_("Invalid encoding in SircPrefs: %1$s"),
                    err->message);
        } else {
            g_free(test);
        }
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
            _("TLS: %1$s, TLS verify certificate: %2$s, Encoding: %3$s"),
            prefs->tls ? t : f, prefs->tls_noverify ? f : t, prefs->encoding);

    char *dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

void sirc_prefs_free(SircPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs->encoding);

    g_free(prefs);
}
