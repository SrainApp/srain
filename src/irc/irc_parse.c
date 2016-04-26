 /**
 * @file irc_parse.c
 * @brief provide function for parsing raw IRC data
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 */

#define __LOG_ON

#include <string.h>
#include "irc.h"
#include "log.h"

/* strip unprintable char and irc color code */
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
 * @brief parsing IRC raw data
 *
 * @param ircbuf a buffer contains ONE IRC raw message (end with "\n\r")
 * @param ircmsg a pointer points to a IRCMsg strcture, used to store parsing result
 *
 * @return message type,
 *      - IRCMSG_MSG: recv a normal message
 *      - IRCMSG_PING/NOTICE/ERROR: serve message that do not need to care by IRC user
 *        (after you recv IRCMSG_ERROR, connection maybe closed by server)
 *      - IRCMSG_UNKNOWN: unrecognized
 */
IRCMsgType irc_parse(char *ircbuf, IRCMsg *ircmsg){
    if (strncmp(ircbuf, "PING :", 6) == 0){
        LOG_FR("PING? PONG");
        return IRCMSG_PING;
    } 
    else if (strncmp(ircbuf, "NOTICE AUTH :", 13) == 0 ){
        // Don't care
        LOG_FR("NOTICE: %s", ircbuf);
        return IRCMSG_NOTICE;
    }
    else if (strncmp(ircbuf, "ERROR :", 7) == 0 ){
        // Still don't care
        LOG_FR("ERROR %s", ircbuf);
        return IRCMSG_ERROR;
    } else {
        /* This is a irc message
         * IRS protocol message format?
         * See: https://tools.ietf.org/html/rfc1459#section-2.3
         */
        char *prefix_ptr, *command_ptr;
        char *trailing_ptr, *middle_ptr;
        char *nick_ptr, *user_ptr, *host_ptr;

        // <message> ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
        // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
        prefix_ptr = strtok(ircbuf + 1, " ");
        command_ptr = strtok(NULL, " ");
        middle_ptr = strtok(NULL, "");
        // FIXME: crash here, see #19
        trailing_ptr = strstr(middle_ptr, " :");

        if (!prefix_ptr || !command_ptr || !middle_ptr) goto bad;
        strncpy(ircmsg->command, command_ptr, COMMAND_LEN);
        LOG_FR("command: {%s}", ircmsg->command);

        // <prefix> ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
        nick_ptr = strtok(prefix_ptr, "!");
        user_ptr = strtok(NULL, "@");
        host_ptr = strtok(NULL, "");
        if (nick_ptr && user_ptr && host_ptr){
            strncpy(ircmsg->nick, nick_ptr, NICK_LEN);
            strncpy(ircmsg->user, user_ptr, USER_LEN);
            strncpy(ircmsg->host, host_ptr, HOST_LEN);
            LOG_FR("nick: %s, user: %s, host: %s", ircmsg->nick, ircmsg->user, ircmsg->host);
        } else {
            strncpy(ircmsg->servername, prefix_ptr, SERVER_LEN);
            LOG_FR("servername: %s", ircmsg->servername);
        }

        // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
        /* if no trailing */
        if (trailing_ptr){
            /* a message may be separated in different irc message */
            strncpy(ircmsg->message, trailing_ptr + 2, MSG_LEN);
            LOG_FR("message: %s", ircmsg->message);
        } else if (middle_ptr[0] == ':'){
            strncpy(ircmsg->message, middle_ptr + 1, MSG_LEN);
            LOG_FR("message(no param): %s", ircmsg->message);
        }

        LOG_F("param: ");
        middle_ptr = strtok(middle_ptr, " ");
        do {
            if (middle_ptr[0] == ':') break;
            strncpy(ircmsg->param[ircmsg->nparam++], middle_ptr, PARAM_LEN);
            LOG("%s(%d) ", ircmsg->param[ircmsg->nparam-1], ircmsg->nparam);
            if (ircmsg->nparam > PARAM_COUNT - 1){
                ERR_FR("too many params: %s", ircbuf);
                return IRCMSG_UNKNOWN;
            }
        } while ((middle_ptr = strtok(NULL, " ")) != NULL);
        LOG("\n");

        strip(ircmsg->message);

        return IRCMSG_MSG;
bad:
        ERR_FR("unrecognized message: %s", ircbuf);
        return IRCMSG_UNKNOWN;
    }
}
