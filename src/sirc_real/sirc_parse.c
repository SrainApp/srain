 /**
 * @file sirc_parse.c
 * @brief Raw IRC data parser
 * @author Shengyu Zhang <silverrain@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 */

#define __LOG_ON
// #define __DBG_ON

#include <string.h>
#include <glib.h>

#include "sirc/sirc.h"
#include "sirc_parse.h"

#include "srain.h"
#include "log.h"

/* strip unprintable char and irc color code */
// TODO: use message filter
static void strip(char *str){
    int i;
    int j;
    int len;

    j = 0;
    len = strlen(str);

    for (i = 0; i < len; i++){
        switch (str[i]){
            case 2: case 0xf: case 0x16:
            case 0x1d: case 0x1f:
                break;
            case 3:  // irc color code
                if (str[i+1] >= '0' && str[i+1] <= '9'){
                    if (str[i+2] >= '0' && str[i+2] <= '9'){
                        i += 2;
                    } else {
                        i += 1;
                    }
                }
                break;
            default:
                str[j++] = str[i];
        }
    }

    str[j] = '\0';
}

/**
 * @brief Parsing IRC raw data
 *
 * @param line A buffer contains ONE IRC raw message (without the trailing "\n\r")
 * @param imsg A pointer points to a pre-allocated IRCMsg strcture, used to
 *               store parsing result
 *
 * @return IRC message type, (OUT-OF-DATA)
 *      - SIRC_MSG_MESSAGE: recv a normal message
 *      - SIRC_MSG_PING/NOTICE/ERROR: serve message that do not need to care by IRC user
 *        (after you recv SIRC_MSG_ERROR, connection maybe closed by server)
 *      - SIRC_MSG_UNKNOWN: unrecognized
 */
int sirc_parse(char *line, SircMessage *imsg){
    memset(imsg, 0, sizeof(SircMessage));

    /* Miscellaneous messages check */
    if (strncmp(line, "PING :", sizeof("PING :") - 1) == 0){
        imsg->msg = line + sizeof("PING :") - 1;
        imsg->type = SIRC_MSG_PING;
        return SRN_OK;
    }
    if (strncmp(line, "PONG :", sizeof("PONG :") - 1) == 0){
        imsg->msg = line + sizeof("PONG :") - 1;
        imsg->type = SIRC_MSG_PONG;
        return SRN_OK;
    }
    else if (strncmp(line, "NOTICE AUTH :", sizeof("NOTICE AUTH :") - 1) == 0){
        imsg->msg = line + sizeof("NOTICE AUTH :") - 1;
        imsg->type = SIRC_MSG_NOTICE;
        return SRN_OK;
    }
    else if (strncmp(line, "ERROR :", sizeof("ERROR :") - 1) == 0 ){
        imsg->msg = line + sizeof("ERROR :") - 1;
        imsg->type = SIRC_MSG_ERROR;
        return SRN_OK;
    } else {
        /* This is a IRC message
         * IRS protocol message format?
         * See: https://tools.ietf.org/html/rfc1459#section-2.3
         */
        char *prefix_ptr, *command_ptr;
        char *trailing_ptr, *middle_ptr;
        char *nick_ptr, *user_ptr, *host_ptr;

        // <message> ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
        // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
        prefix_ptr = strtok(line + 1, " ");
        command_ptr = strtok(NULL, " ");
        middle_ptr = strtok(NULL, "");

        if (!prefix_ptr || !command_ptr || !middle_ptr) goto bad;
        trailing_ptr = strstr(middle_ptr, " :");

        imsg->cmd = command_ptr;
        DBG_FR("command: %s", imsg->cmd);

        // <prefix> ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
        imsg->prefix = prefix_ptr;
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

        // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
        /* If no trailing */
        if (trailing_ptr){
            /* A message may be separated in different irc message */
            *trailing_ptr = '\0';   // Prevent influence by param parse
            imsg->msg = trailing_ptr + 2;
            DBG_FR("message: %s", imsg->msg);
        } else if (middle_ptr[0] == ':'){
            imsg->msg = middle_ptr + 1;
            DBG_FR("message(no param): %s", imsg->msg);
        }

        DBG_F("param: ");
        middle_ptr = strtok(middle_ptr, " ");
        do {
            if (middle_ptr[0] == ':') break;
            imsg->params[imsg->nparam++] = middle_ptr;
            DBG("%s(%d) ", imsg->params[imsg->nparam-1], imsg->nparam);
            if (imsg->nparam > SIRC_PARAM_COUNT - 1){
                ERR_FR("Too many params: %s", line);
                goto bad;
            }
        } while ((middle_ptr = strtok(NULL, " ")) != NULL);
        DBG("\n");

        // strip(imsg->msg); // TODO
        imsg->type = SIRC_MSG_MESSAGE;

        return SRN_OK;
bad:
        ERR_FR("Unrecognized message: %s", line);
        return SRN_ERR;
    }
}
