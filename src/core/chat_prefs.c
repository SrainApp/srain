/* Copyright (C) 2016-2018 Shengyu Zhang <srain@srain.im>
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

#include "server.h"
#include "sui/sui.h"
#include "ret.h"
#include "i18n.h"

ChatPrefs *chat_prefs_new(){
    ChatPrefs *prefs;

    prefs = g_malloc0(sizeof(ChatPrefs));
    prefs->ui = sui_prefs_new();

    return prefs;
}

SrnRet chat_prefs_check(ChatPrefs *prefs){
    if (!prefs){
        return RET_ERR(_("Invalid ChatPrefs instance"));
    }
    return sui_prefs_check(prefs->ui);
}

void chat_prefs_free(ChatPrefs *prefs){
    g_return_if_fail(prefs);

    sui_prefs_free(prefs->ui);
    g_free(prefs);
}
