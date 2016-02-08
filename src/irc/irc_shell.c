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

int irc_connect(IRC *irc, const char *server, const char *port){
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

int irc_login(IRC *irc, const char* nick){
    LOG_FR("attemping to login as %s", nick);

    return irc_reg(irc->fd, nick, "Srain", "EL PSY CONGROO");
}

int irc_join_chan(IRC *irc, const char* chan){
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

int irc_leave_chan(IRC *irc, const char *chan){
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

int irc_send(IRC *irc, const char *chan, const char *msg){
    return irc_msg(irc->fd, chan, msg);
}

int irc_parse(IRC *irc, char *nick, char *chan, char *cmd, char *msg){
    char *nick_ptr, *user_ptr, *chan_ptr, *msg_ptr, *host_ptr, *cmd_ptr;

    if (strncmp(irc->servbuf, "PING :", 6) == 0){
        LOG_FR("PING? PONG");
        return irc_pong(irc->fd, &irc->servbuf[6]);
    } 
    else if (strncmp(irc->servbuf, "NOTICE AUTH :", 13) == 0 ){
        // Don't care
        LOG_FR("NOTICE: %s", irc->servbuf);
        return 0;
    }
    else if (strncmp(irc->servbuf, "ERROR :", 7) == 0 ){
        // Still don't care
        LOG_FR("ERROR %s", irc->servbuf);
        return 0;
    } else {
        // See: https://tools.ietf.org/html/rfc1459#section-2.3
        *nick = *chan = *cmd = *msg = '\0';

        // Checks if we have non-message string
        if (strchr(irc->servbuf, 1) != NULL){
            return 0;
        }
        nick_ptr = strtok(irc->servbuf + 1, "!");
        user_ptr = strtok(NULL, "@");
        /* is prefix a server message */
        if (!user_ptr){
            user_ptr = strtok(irc->servbuf + 1, ":");
            /* ignore <servername>, <command> and <middle> */
            msg_ptr = strtok(NULL, ":");
            if (msg_ptr) strncpy(msg, msg_ptr, MSG_LEN);
            return 0;
        }
        host_ptr = strtok(NULL, " ");
        cmd_ptr = strtok(NULL, " ");
        chan_ptr = strtok(NULL, " ");
        msg_ptr = strtok(NULL, "");

        if (nick_ptr) strncpy(nick, nick_ptr, NICK_LEN);
        if (msg_ptr) strncpy(msg, msg_ptr + 1, MSG_LEN);
        if (chan_ptr) strncpy(chan, chan_ptr, CHAN_LEN);
        if (cmd_ptr) strncpy(cmd, cmd_ptr, 32); // how long?
        return 0;
    }
}

int irc_recv(IRC *irc, char *nick, char *chan, char *cmd, char *msg){
    char tmpbuf[512];
    int rc, i;

    if ((rc = sck_recv(irc->fd, tmpbuf, sizeof(tmpbuf) - 2 )) <= 0 ){
        ERR_FR("buffer overflow");
        return -1;
    }

    tmpbuf[rc] = '\0';
    // LOG("{ %s }", tmpbuf);

    *nick = *chan = *cmd = *msg = '\0';
    char *msgoff = msg;
    for (i = 0; i < rc; ++i ){
        switch (tmpbuf[i]){
            /* a respone may include one or more \r\n */
            case '\r': break;
            case '\n': {
                           irc->servbuf[irc->bufptr] = '\0';
                           irc->bufptr = 0;
                           if (irc_parse(irc, nick, chan, cmd, msgoff) == 0){
                               int i = strlen(msgoff);
                               msgoff[i] = '\n';
                               msgoff += i + 1;
                               break;
                           } else {
                               return -1;
                           }
                       }
            default: {
                         irc->servbuf[irc->bufptr] = tmpbuf[i];
                         if (irc->bufptr >= (sizeof (irc->servbuf) -1))
                             // Overflow!
                             ;
                         else
                             irc->bufptr++;
                     }
        }
    }
    LOG_FR("nick = %s chan = %s cmd = %s msg = \n%s", nick, chan, cmd, msg);
    return 0;
}


void irc_close(IRC *irc){
    close(irc->fd);
}
