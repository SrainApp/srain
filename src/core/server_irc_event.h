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

#ifndef __IRC_EVENT_H
#define __IRC_EVENT_H

#include "sirc/sirc.h"

void server_irc_event_connect(SircSession *sirc, const char *event);

void server_irc_event_disconnect(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_ctcp_req(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count);

void server_irc_event_ctcp_rsp(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void server_irc_event_ping(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void server_irc_event_pong(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void server_irc_event_error(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void server_irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char *params[], int count);

void server_irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char *params[], int count);


#endif /* __IRC_EVENT_H */
