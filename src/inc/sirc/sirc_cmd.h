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

#ifndef __SIRC_CMD_H
#define __SIRC_CMD_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

/* IRC Protocol */
int sirc_cmd_ping(SircSession *sirc, const char *data);
int sirc_cmd_pong(SircSession *sirc, const char *data);
int sirc_cmd_join(SircSession *sirc, const char *chan, const char *passwd);
int sirc_cmd_user(SircSession *sirc, const char *username, const char *hostname, const char *servername, const char *realname);
int sirc_cmd_part(SircSession *sirc, const char *chan, const char *reason);
int sirc_cmd_nick(SircSession *sirc, const char *nick);
int sirc_cmd_quit(SircSession *sirc, const char *reason);
int sirc_cmd_topic(SircSession *sirc, const char *chan, const char *topic);
int sirc_cmd_action(SircSession *sirc, const char *target, const char *msg);
int sirc_cmd_msg(SircSession *sirc, const char *target, const char *msg);
int sirc_cmd_whois(SircSession *sirc, const char *nick);
int sirc_cmd_names(SircSession *sirc, const char *chan);
int sirc_cmd_invite(SircSession *sirc, const char *nick, const char *chan);
int sirc_cmd_kick(SircSession *sirc, const char *nick, const char *chan, const char *reason);
int sirc_cmd_mode(SircSession *sirc, const char *target, const char *mode);
int sirc_cmd_pass(SircSession *sirc, const char *pass);
int sirc_cmd_list(SircSession *sirc, const char *chan, const char *target);
int sirc_cmd_ctcp_req(SircSession *sirc, const char *target, const char *cmd, const char *msg);
int sirc_cmd_ctcp_rsp(SircSession *sirc, const char *target, const char *cmd, const char *msg);
int sirc_cmd_cap_ls(SircSession *sirc, const char *version);
int sirc_cmd_cap_list(SircSession *sirc);
int sirc_cmd_cap_req(SircSession *sirc, const char *caps);
int sirc_cmd_cap_end(SircSession *sirc);
int sirc_cmd_authenticate(SircSession *sirc, const char *msg);
int sirc_cmd_raw(SircSession *sirc, const char *fmt, ...);

#endif /* __IRC_CMD_H */
