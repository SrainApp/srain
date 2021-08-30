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

/* https://ircv3.net/specs/extensions/message-tags#size-limit */
#define TAGS_SIZE_LIMIT 8191

SircMessage *sirc_message_new(){
    return g_malloc0(sizeof(SircMessage));
}

void sirc_message_free(SircMessage *imsg){
    str_assign(&imsg->prefix, NULL);
    str_assign(&imsg->nick, NULL);
    str_assign(&imsg->user, NULL);
    str_assign(&imsg->host, NULL);
    str_assign(&imsg->cmd, NULL);

    for (int i = 0; i < imsg->nparam; i++){
        str_assign(&imsg->params[i], NULL);
    }

    for (int i = 0; i < imsg->ntags; i++){
        str_assign(&imsg->tags[i].key, NULL);
        str_assign(&imsg->tags[i].value, NULL);
    }

    if (imsg->ntags > 0) {
        g_free(imsg->tags);
    }

    g_free(imsg);
}

void sirc_message_transcoding(SircMessage *imsg, const char *from_codeset) {
    str_transcoding(&imsg->prefix, from_codeset);
    str_transcoding(&imsg->nick, from_codeset);
    str_transcoding(&imsg->user, from_codeset);
    str_transcoding(&imsg->host, from_codeset);
    str_transcoding(&imsg->cmd, from_codeset);

    for (int i = 0; i < imsg->nparam; i++){
        str_transcoding(&imsg->params[i], from_codeset);
    }

    /* No need to transcode tags, they are guaranteed to be UTF-8 */
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
     * IRC protocol message format?
     * See: https://ircv3.net/specs/extensions/message-tags
     */
    char *rfc1459_line;
    char *tags_ptr;
    char *prefix_ptr, *command_ptr;
    char *trailing_ptr, *params_ptr;
    char *nick_ptr, *user_ptr, *host_ptr;

    // <message> ::= ['@' <tags> <SPACE> ] [':' <prefix> <SPACE> ] <command> <params> <crlf>
    if (line[0] == '@'){
        size_t ntags;
        tags_ptr = line + 1;
        rfc1459_line = strchr(line, ' ') + 1;

        /* Count the number of tags to allocate a tag array */
        ntags = 1;
        for (char *p=tags_ptr; *p != ' '; p++){
            if (*p == ';'){
                ntags++;
            }
        }

        imsg->ntags = ntags;
        imsg->tags = g_malloc_n(ntags, sizeof(SircMessageTag));
        size_t i=0;
        char current_tag_key[TAGS_SIZE_LIMIT];
        char current_tag_value[TAGS_SIZE_LIMIT];
        char *current_tag_key_ptr = current_tag_key;
        char *current_tag_value_ptr = current_tag_value;
        gboolean in_key = TRUE;
        for (char *p=tags_ptr; ; p++){
            if (*p == ';' || *p == ' '){
                /* next tag or end of tags*/
                in_key = TRUE;
                *current_tag_key_ptr = '\0';
                imsg->tags[i].key = strdup(current_tag_key);
                if (current_tag_value == current_tag_value_ptr){
                    /* Key is absent or empty */
                    imsg->tags[i].value = NULL;
                }
                else {
                    *current_tag_value_ptr = '\0';
                    imsg->tags[i].value = strdup(current_tag_value);
                }
                if (*p == ' '){
                    /* end of tags */
                    break;
                }
                i++;

                current_tag_key_ptr = current_tag_key;
                current_tag_value_ptr = current_tag_value;
            }
            else if (*p == '=' && in_key){
                /* tag's value */
                in_key = FALSE;
            }
            else if (in_key){
                *(current_tag_key_ptr++) = *p;
            }
            else if (!in_key && *p == '\\'){
                /* Possibly an escaped character in the value */
                p++;
                if (*p == ':')
                    *(current_tag_value_ptr++) = ';';
                else if (*p == 's')
                    *(current_tag_value_ptr++) = ' ';
                else if (*p == '\\')
                    *(current_tag_value_ptr++) = '\\';
                else if (*p == 'r')
                    *(current_tag_value_ptr++) = '\r';
                else if (*p == 'n')
                    *(current_tag_value_ptr++) = '\n';
                else{
                    /* "If a \ exists with no valid escape character (for example, \b),
                     * then the invalid backslash SHOULD be dropped.
                     * For example, \b should unescape to just b."
                     */
                    *(current_tag_value_ptr++) = *p;
                }
            }
            else{
                *(current_tag_value_ptr++) = *p;
            }
        }
    }
    else{
        imsg->tags = NULL;
        imsg->ntags = 0;
        rfc1459_line = line;
    }

    /* Now parse like in RFC1459 */

    if (rfc1459_line[0] == ':'){
        prefix_ptr = strtok(rfc1459_line + 1, " "); // Skip ':'
        command_ptr = strtok(NULL, " ");
    } else {
        prefix_ptr = NULL;
        command_ptr = strtok(rfc1459_line, " ");
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
        imsg->prefix = g_strdup("");
    }

    // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
    /* NOTE: After extracting the parameter list, all parameters are equal
     *       whether matched by <middle> or <trailing>. <trailing> is just a
     *       syntactic trick to allow SPACE within the parameter. (RFC 2812)
     */

    if (params_ptr[0] == ':'){
        /* params have only one element, it is a trailing */
        trailing_ptr = params_ptr + 1;
    } else {
        trailing_ptr = strstr(params_ptr, " :");
        if (trailing_ptr){
            /* trailing exists in params */
            *trailing_ptr = '\0';   // Prevent influenced from split params
            trailing_ptr = trailing_ptr + 2;
        }

        /* Split params which don't contain trailing */
        DBG_F("params: ");
        params_ptr = strtok(params_ptr, " ");
        do {
            if (imsg->nparam >= SIRC_PARAM_COUNT){
                ERR_FR("Too many params: %s", line);
                goto bad;
            }
            imsg->params[imsg->nparam++] = g_strdup(params_ptr);
            DBG("%s(%d) ", imsg->params[imsg->nparam-1], imsg->nparam);
        } while ((params_ptr = strtok(NULL, " ")) != NULL);
        DBG("\n");
    }

    if (trailing_ptr) {
        if (imsg->nparam >= SIRC_PARAM_COUNT){
            ERR_FR("Too many params: %s", line);
            goto bad;
        }
        imsg->params[imsg->nparam++] = g_strdup(trailing_ptr);
        DBG_FR("trailing: %s", imsg->params[imsg->nparam-1]);
    }

    return imsg;
bad:
    ERR_FR("Unrecognized message: %s", line);
    sirc_message_free(imsg);

    return NULL;
}
