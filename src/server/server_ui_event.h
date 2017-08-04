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

#ifndef __UI_EVENT_H
#define __UI_EVENT_H

#include "sui/sui.h"

void server_ui_event_activate(SuiEvent event, const char *params[], int count);
void server_ui_event_connect(SuiEvent event, const char *params[], int count);

void server_ui_event_disconnect(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_whois(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_ignore(SuiSession *sui, SuiEvent event, const char *params[], int count);
void server_ui_event_cutover(SuiSession *sui, SuiEvent event, const char *params[], int count);

#endif /* __UI_EVENT_H */
