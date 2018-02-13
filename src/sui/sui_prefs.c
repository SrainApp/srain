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

#include "sui/sui.h"
#include "i18n.h"
#include "utils.h"

SuiAppPrefs *sui_app_prefs_new(){
    SuiAppPrefs *prefs;

    prefs = g_malloc0(sizeof(SuiAppPrefs));

    return prefs;
}

SrnRet sui_app_prefs_check(SuiAppPrefs *prefs){
    const char *fmt = _("Missing field in SuiAppPrefs: %1$s");

    if (!prefs){
        return RET_ERR(_("Invalid SuiAppPrefs instance"));
    }
    if (str_is_empty(prefs->theme)){
        return RET_ERR(fmt, "theme");
    }
    return SRN_OK;
}

void sui_app_prefs_free(SuiAppPrefs *prefs){
    g_return_if_fail(prefs);

    if (prefs->theme){
        g_free(prefs->theme);
        prefs->theme = NULL;
    }

    g_free(prefs);
}

SuiPrefs *sui_prefs_new(){
    SuiPrefs *prefs;

    prefs = g_malloc0(sizeof(SuiPrefs));

    return prefs;
}

SrnRet sui_prefs_check(SuiPrefs *prefs){
    if (!prefs){
        return RET_ERR(_("Invalid SuiPrefs instance"));
    }
    return SRN_OK;
}

void sui_prefs_free(SuiPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
