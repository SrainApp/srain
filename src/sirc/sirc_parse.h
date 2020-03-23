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

#ifndef __SIRC_PARSE_H
#define __SIRC_PARSE_H

#define SIRC_PARAM_COUNT    64      // RFC 2812 limits it to 14

typedef struct {
    char *prefix; // servername or nick!user@host
    char *nick, *user, *host;

    char *cmd;
    int nparam;
    char *params[SIRC_PARAM_COUNT];  // middle and trailing
} SircMessage;

SircMessage *sirc_message_new();
void sirc_message_free(SircMessage *imsg);
void sirc_message_transcoding(SircMessage *imsg, const char *from_codeset);
SircMessage *sirc_parse(char *line);

#endif /* __SIRC_PARSE_H */
