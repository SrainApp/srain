/* socket.c
 * This file is modified from <https://github.com/Themaister/simple-irc-bot>
 */
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "socket.h"
#include "irc.h"
#include "irc_core.h"
#include "irc_parse.h"
#include "log.h"

int irc_connect(irc_t *irc, const char *server, const char *port){
    int i;

    LOG_FR("connecting to %s:%s", server, port);
    if ((irc->fd =  get_socket(server, port)) < 0){
        ERR_FR("failed to connect %s:%s", server, port);
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
    memset(ircmsg, 0, sizeof(irc_msg_t));

    // LOG_FR("tmpbuf_ptr = %d, rc = %d", tmpbuf_ptr, rc);
    irc_msg_type_t res = IRCMSG_UNKNOWN;
    for (i = tmpbuf_ptr; i < rc; i++){
        switch (tmpbuf[i]){
            /* a respone may include one or more \r\n */
            case '\r': break;
            case '\n': {
                           irc->servbuf[irc->bufptr] = '\0';
                           irc->bufptr = 0;
                           res = irc_parse(irc->servbuf, ircmsg, 0);
                           switch (res){
                               case IRCMSG_PING:
                                   irc_pong(irc->fd, irc->servbuf);
                                   break;
                               default:
                                   break;
                           }
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
