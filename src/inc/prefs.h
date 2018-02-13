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

#ifndef __PREFS_H
#define __PREFS_H

#include "srain.h"
#include "server.h"
#include "log.h"
#include "ret.h"

void prefs_init();
SrnRet prefs_read();
void prefs_finalize();

SrnRet prefs_read_log_prefs(LogPrefs *prefs);
SrnRet prefs_read_sui_app_prefs(SuiAppPrefs *prefs);
SrnRet prefs_read_server_prefs_list();
SrnRet prefs_read_server_prefs(ServerPrefs *prefs);
SrnRet prefs_read_chat_prefs(ChatPrefs *prefs, const char *srv_name, const char *chat_name);
SrnRet prefs_read_sirc_prefs(SircPrefs *prefs, const char *srv_name);
SrnRet prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name, const char *chat_name);

#endif /*__PREFS_H */
