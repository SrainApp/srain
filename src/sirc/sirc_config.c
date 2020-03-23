/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
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

SircConfig* sirc_config_new(){
    SircConfig *cfg;

    cfg = g_malloc0(sizeof(SircConfig));

    return cfg;
}

SrnRet sirc_config_check(SircConfig *cfg){
    // const char *fmt = _("Missing field in IRC config: %1$s");

    if (!cfg){
        return RET_ERR(_("Invalid IRC config instance"));
    }

    if (str_is_empty(cfg->encoding)) {
        str_assign(&cfg->encoding, SRN_CODESET);
    }

    /* Check encoding */
    {
        char *test;
        GError *err = NULL;

        // Just to test whether the codeset valid
        test = g_convert("", -1, SRN_CODESET, cfg->encoding, NULL, NULL, &err);
        if (err){
            return RET_ERR(_("Invalid encoding in IRC config: %1$s"),
                    err->message);
        }
        g_free(test);
    }

    return SRN_OK;
}

char* sirc_config_dump(SircConfig *cfg){
    GString *str;
    g_return_val_if_fail(cfg, NULL);

    const char *t = _("True");
    const char *f = _("False");

    str = g_string_new("");
    g_string_append_printf(str,
            _("TLS: %1$s, TLS verify certificate: %2$s, Encoding: %3$s"),
            cfg->tls ? t : f, cfg->tls_noverify ? f : t, cfg->encoding);

    char *dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

void sirc_config_free(SircConfig *cfg){
    g_return_if_fail(cfg);

    g_free(cfg->encoding);

    g_free(cfg);
}
