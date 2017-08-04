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

#ifndef __SIRC_PARSE_H
#define __SIRC_PARSE_H

#define SIRC_CMD_LEN        64      // unconfirm
#define SIRC_PARAM_COUNT    64      // unconfirm
#define SIRC_PARAM_LEN      64      // unconfirm
#define SIRC_MSG_LEN        512     // unconfirm

typedef enum {
    /* Standard IRC messages */
    SIRC_MSG_MESSAGE, // Unused
} SircMessageType;

typedef struct {
    const char *prefix; // servername or nick!user@host
    const char *nick, *user, *host;

    const char *cmd;
    int nparam;
    const char *params[SIRC_PARAM_COUNT];  // middle
    const char *msg;  // trailing
    SircMessageType type;

    void *ctx;   // irc server this message belongs to
} SircMessage;

int sirc_parse(char *line, SircMessage *imsg);

#endif /* __SIRC_PARSE_H */
