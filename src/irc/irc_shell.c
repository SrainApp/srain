/* socket.c
 * This file is modified from <https://github.com/Themaister/simple-irc-bot>
 */
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "socket.h"
#include "irc_core.h"
#include "irc.h"
#include "log.h"

int irc_connect(irc_t *irc, const char *server, const char *port){
    int i;

    LOG_FR("connecting to %s:%s", server, port);
    if ((irc->fd =  get_socket(server, port)) < 0){
        return -1;
    }
    irc->bufptr = 0;
    irc->nchan = 0;
    for (i = 0; i < CHAN_NUM; i++){
        irc->chans[i][0] = '\0';
    }

    return 0;
}

int irc_login(irc_t *irc, const char* nick){
    LOG_FR("attemping to login as %s", nick);

    return irc_reg(irc->fd, nick, "Srain", "EL PSY CONGROO");
}

int irc_join_chan(irc_t *irc, const char* chan){
    int i;

    LOG_FR("attemping to join %s", chan);
    for (i = 0; i < CHAN_NUM; i++){
        if (strlen(irc->chans[i]) == 0){
            strncpy(irc->chans[i], chan, CHAN_LEN - 2);
            irc->chans[i][CHAN_LEN-1] = '\0';

            return irc_join(irc->fd, chan);
        }
    }

    ERR_FR("channels list is full");
    return -1;
}

int irc_leave_chan(irc_t *irc, const char *chan){
    int i;

    for (i = 0; i < irc->nchan; i++){
        if (strncmp(irc->chans[i], chan, CHAN_LEN) == 0){
            if (irc_part(irc->fd, irc->chans[i]) == 0){
                memset(irc->chans[i], 0, CHAN_LEN);
                return 0;
            } else {
                return -1;
            }
        }
    }

    ERR_FR("no such channel %s", chan);
    return -1;
}

int irc_send(irc_t *irc, const char *chan, const char *msg){
    return irc_msg(irc->fd, chan, msg);
}

irc_msg_type_t irc_parse(irc_t *irc, irc_msg_t *ircmsg, int msgoff){
    if (strncmp(irc->servbuf, "PING :", 6) == 0){
        LOG_FR("PING? PONG");
        irc_pong(irc->fd, &irc->servbuf[6]);
        return IRCMSG_PING;
    } 
    else if (strncmp(irc->servbuf, "NOTICE AUTH :", 13) == 0 ){
        // Don't care
        LOG_FR("NOTICE: %s", irc->servbuf);
        return IRCMSG_NOTICE;
    }
    else if (strncmp(irc->servbuf, "ERROR :", 7) == 0 ){
        // Still don't care
        LOG_FR("ERROR %s", irc->servbuf);
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
        prefix_ptr = strtok(irc->servbuf + 1, " ");
        command_ptr = strtok(NULL, " ");
        middle_ptr = strtok(NULL, "");
        trailing_ptr = strstr(middle_ptr, " :");
        if (!prefix_ptr || !command_ptr || !middle_ptr) goto bad;
        strncpy(ircmsg->command, command_ptr, COMMAND_LEN);
        LOG_FR("command: %s", ircmsg->command);

        // <prefix> ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
        nick_ptr = strtok(prefix_ptr, "!");
        user_ptr = strtok(NULL, "@");
        host_ptr = strtok(NULL, "");
        if (nick_ptr && user_ptr && host_ptr){
            strncpy(ircmsg->nick, nick_ptr, NICK_LEN);
            strncpy(ircmsg->user, user_ptr, USER_LEN);
            strncpy(ircmsg->host, host_ptr, HOST_LEN);
            // LOG_FR("nick: %s, user: %s, host: %s", ircmsg->nick, ircmsg->user, ircmsg->host);
        } else {
            strncpy(ircmsg->servername, prefix_ptr, SERVER_LEN);
            // LOG_FR("servername: %s", ircmsg->servername);
        }

        // <params> ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
        /* if no trailing */
        if (trailing_ptr){
            /* a message may be separated in different irc message */
            strncpy(ircmsg->message, trailing_ptr + 2, MSG_LEN);
            // LOG_FR("part of message: %s", ircmsg->message);
        }

        // LOG_F("param: ");
        middle_ptr = strtok(middle_ptr, " ");
        do {
            if (middle_ptr[0] == ':') break;
            strncpy(ircmsg->param[ircmsg->nparam++], middle_ptr, PRARM_LEN);
            // LOG("%s(%d) ", ircmsg->param[ircmsg->nparam-1], ircmsg->nparam);
            if (ircmsg->nparam > PRARM_COUNT - 1){
                ERR_FR("too many params: %s", irc->servbuf);
                return IRCMSG_UNKNOWN;
            }
        } while ((middle_ptr = strtok(NULL, " ")) != NULL);
        // LOG("\n")

        return IRCMSG_MSG;
bad:
        ERR_FR("unrecognized message: %s", irc->servbuf);
        return IRCMSG_UNKNOWN;
    }
}

irc_msg_type_t irc_recv(irc_t *irc, irc_msg_t *ircmsg){
    static int tmpbuf_ptr = 0;
    static char tmpbuf[BUF_LEN];
    static int rc;
    int i;

    if (tmpbuf_ptr == 0){
        if ((rc = sck_recv(irc->fd, tmpbuf, BUF_LEN -2)) <= 0 ){
            ERR_FR("socket error");
            return IRCMSG_UNKNOWN;
        }
        tmpbuf[rc] = '\0';
        // LOG("{\n%s}\n", tmpbuf);
    }

    // LOG_FR("tmpbuf_ptr = %d, rc = %d", tmpbuf_ptr, rc);
    irc_msg_type_t res = IRCMSG_UNKNOWN;
    for (i = tmpbuf_ptr; i < rc; i++){
        switch (tmpbuf[i]){
            /* a respone may include one or more \r\n */
            case '\r': break;
            case '\n': {
                           irc->servbuf[irc->bufptr] = '\0';
                           irc->bufptr = 0;
                           res = irc_parse(irc, ircmsg, 0);
                           tmpbuf_ptr = i + 1;
                           return res;
                       }
            default: {
                         irc->servbuf[irc->bufptr] = tmpbuf[i];
                         if (irc->bufptr >= (sizeof(irc->servbuf) - 1)){
                             ERR_FR("irc buffer overflow");
                         } else irc->bufptr++;
                     }
        }
    }

    tmpbuf_ptr = 0;
    return IRCMSG_UNKNOWN;
}

void irc_close(irc_t *irc){
    close(irc->fd);
    LOG_FR("closed");
}
