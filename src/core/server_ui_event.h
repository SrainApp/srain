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

#ifndef __UI_EVENT_H
#define __UI_EVENT_H

#include "sui/sui.h"

SrnRet srn_server_ui_event_open(SuiApplication *app, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_activate(SuiApplication *app, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_shutdown(SuiApplication *app, SuiEvent event, GVariantDict *params);

SrnRet srn_server_ui_event_connect(SuiWindow *win, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_server_list(SuiWindow *win, SuiEvent event, GVariantDict *params);

SrnRet srn_server_ui_event_disconnect(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_quit(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_send(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_join(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_part(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_query(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_unquery(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_kick(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_invite(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_whois(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_ignore(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_cutover(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
SrnRet srn_server_ui_event_chan_list(SuiBuffer *sui, SuiEvent event, GVariantDict *params);

#endif /* __UI_EVENT_H */
