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
 * @version 0.06.2
 * @date 2016-03-01
 *
 */

#include <string.h>
#include <glib.h>

#include "sirc/sirc.h"
#include "sirc_parse.h"

#include "srain.h"
#include "log.h"
#include "utils.h"

SircMessage *sirc_message_new(){
    return g_malloc0(sizeof(SircMessage));
}

void sirc_message_free(SircMessage *imsg){
    str_assign(&imsg->prefix, NULL);
    str_assign(&imsg->nick, NULL);
    str_assign(&imsg->user, NULL);
    str_assign(&imsg->host, NULL);
    str_assign(&imsg->cmd, NULL);
    str_assign(&imsg->msg, NULL);

    for (int i = 0; i < imsg->nparam; i++){
        str_assign(&imsg->params[i], NULL);
    }

    g_free(imsg);
}

// FIXME: how to use fallback?
void sirc_message_transcoding(SircMessage *imsg,
        const char *to, const char *from, const char *fallback){
    str_transcoding(&imsg->prefix, to, from, fallback);
    str_transcoding(&imsg->nick, to, from, fallback);
    str_transcoding(&imsg->user, to, from, fallback);
    str_transcoding(&imsg->host, to, from, fallback);
    str_transcoding(&imsg->cmd, to, from, fallback);
    str_transcoding(&imsg->msg, to, from, fallback);

    for (int i = 0; i < imsg->nparam; i++){
        str_transcoding(&imsg->params[i], to, from, fallback);
    }
}

/**
 * @brief Parsing IRC raw data
 *
 * @param line A buffer contains ONE IRC raw message (without the trailing "\r\n")
 *
 * @return A SircMessage structure
 */
SircMessage* sirc_parse(char *line){
    SircMessage *imsg;

    DBG_FR("raw: %s", line);

    imsg = sirc_message_new();

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
    imsg->cmd = g_strdup(command_ptr);
    DBG_FR("command: %s", imsg->cmd);

    if (prefix_ptr){
        imsg->prefix = g_strdup(prefix_ptr);
        // <prefix> ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
        nick_ptr = strtok(prefix_ptr, "!");
        user_ptr = strtok(NULL, "@");
        host_ptr = strtok(NULL, "");
        if (nick_ptr && user_ptr && host_ptr){
            imsg->nick = g_strdup(nick_ptr);
            imsg->user = g_strdup(user_ptr);
            imsg->host = g_strdup(host_ptr);
            DBG_FR("nick: %s, user: %s, host: %s", imsg->nick, imsg->user, imsg->host);
        } else {
            DBG_FR("servername: %s", imsg->prefix);
        }
    } else {
        imsg->prefix = strdup("");
    }

    // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]

    if (params_ptr[0] == ':'){
        /* params have only one element, it is a trailing */

        imsg->msg = g_strdup(params_ptr + 1);
        imsg->nparam = 0;
    } else {
        trailing_ptr = strstr(params_ptr, " :");
        if (trailing_ptr){
            /* trailing exists in params */
            *trailing_ptr = '\0';   // Prevent influenced from split params
            imsg->msg = g_strdup(trailing_ptr + 2);
        }

        /* Split params which don't contain trailing */
        DBG_F("params: ");
        params_ptr = strtok(params_ptr, " ");
        do {
            imsg->params[imsg->nparam++] = g_strdup(params_ptr);
            DBG("%s(%d) ", imsg->params[imsg->nparam-1], imsg->nparam);
            if (imsg->nparam > SIRC_PARAM_COUNT - 1){
                ERR_FR("Too many params: %s", line);
                goto bad;
            }
        } while ((params_ptr = strtok(NULL, " ")) != NULL);
        DBG("\n");
    }

    DBG_FR("message: %s", imsg->msg);

    return imsg;
bad:
    ERR_FR("Unrecognized message: %s", line);
    sirc_message_free(imsg);

    return NULL;
}
