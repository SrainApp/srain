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

#ifndef __SUI_EVENT_HDR_H
#define __SUI_EVENT_HDR_H

#include "srain_app.h"
#include "srain_window.h"
#include "srain_buffer.h"

SrnRet sui_application_event_hdr(SuiApplication *app, SuiEvent event, GVariantDict *params);
SrnRet sui_window_event_hdr(SuiWindow *win, SuiEvent event, GVariantDict *params);
SrnRet sui_buffer_event_hdr(SuiBuffer *buf, SuiEvent event, GVariantDict *params);

#endif /* __SUI_EVENT_HDR_H */
