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

 /**
 * @file sirc_parse.c
 * @brief Raw IRC data parser
 * @author Shengyu Zhang <silverrain@outlook.com>
 * @version 0.06.1
 * @date 2016-03-01
 *
 */

#include <string.h>
#include <glib.h>

#include "sirc/sirc.h"
#include "sirc_parse.h"

#include "srain.h"
#include "log.h"

/**
 * @brief Parsing IRC raw data
 *
 * @param line A buffer contains ONE IRC raw message (without the trailing "\r\n")
 * @param imsg A pointer points to a pre-allocated SircIrcMessage strcture,
 *             used to store parsed result
 *
 * @return SRN_OK if no error occurred.
 */
int sirc_parse(char *line, SircMessage *imsg){
    memset(imsg, 0, sizeof(SircMessage));

    DBG_FR("raw: %s", line);

    /* This is a IRC message
     * IRS protocol message format?
     * See: https://tools.ietf.org/html/rfc1459#section-2.3
     */
    char *prefix_ptr, *command_ptr;
    char *trailing_ptr, *params_ptr;
    char *nick_ptr, *user_ptr, *host_ptr;

    // <message> ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
    if (line[0] == ':'){
        prefix_ptr = strtok(line + 1, " "); // Skip ':'
        command_ptr = strtok(NULL, " ");
    } else {
        prefix_ptr = NULL;
        command_ptr = strtok(line, " ");
    }

    params_ptr = strtok(NULL, "");

    if (!command_ptr || !params_ptr) goto bad;
    imsg->cmd = command_ptr;
    DBG_FR("command: %s", imsg->cmd);

    if (prefix_ptr){
        imsg->prefix = prefix_ptr;
        // <prefix> ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
        nick_ptr = strtok(prefix_ptr, "!");
        user_ptr = strtok(NULL, "@");
        host_ptr = strtok(NULL, "");
        if (nick_ptr && user_ptr && host_ptr){
            imsg->nick = nick_ptr;
            imsg->user = user_ptr;
            imsg->host = host_ptr;
            DBG_FR("nick: %s, user: %s, host: %s", imsg->nick, imsg->user, imsg->host);
        } else {
            DBG_FR("servername: %s", imsg->prefix);
        }
    } else {
        imsg->prefix = "";
    }

    // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]

    if (params_ptr[0] == ':'){
        /* params have only one element, it is a trailing */

        imsg->msg = params_ptr + 1;
        imsg->nparam = 0;
    } else {
        trailing_ptr = strstr(params_ptr, " :");
        if (trailing_ptr){
            /* trailing exists in params */
            *trailing_ptr = '\0';   // Prevent influenced from split params
            imsg->msg = trailing_ptr + 2;
        }

        /* Split params which don't contain trailing */
        DBG_F("params: ");
        params_ptr = strtok(params_ptr, " ");
        do {
            imsg->params[imsg->nparam++] = params_ptr;
            DBG("%s(%d) ", imsg->params[imsg->nparam-1], imsg->nparam);
            if (imsg->nparam > SIRC_PARAM_COUNT - 1){
                ERR_FR("Too many params: %s", line);
                goto bad;
            }
        } while ((params_ptr = strtok(NULL, " ")) != NULL);
        DBG("\n");
    }

    DBG_FR("message: %s", imsg->msg);

    return SRN_OK;
bad:
    ERR_FR("Unrecognized message: %s", line);
    return SRN_ERR;
}
