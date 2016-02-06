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

int irc_parse(IRC *irc, char *irc_nick, char *irc_msg){
    if (strncmp(irc->servbuf, "PING :", 6) == 0){
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
    }

    // Here be lvl. 42 dragonn boss
    // Parses IRC message that pulls out nick and message. 
    else {
        char *ptr;
        int privmsg = 0;

        *irc_nick = '\0';
        *irc_msg = '\0';

        // Checks if we have non-message string
        if (strchr(irc->servbuf, 1) != NULL){
            return 0;
        }

        if (irc->servbuf[0] == ':'){
            ptr = strtok(irc->servbuf, "!");
            if (ptr == NULL){
                LOG_FR("unkown message");
                return 0;
            } else {
                strncpy(irc_nick, &ptr[1], NICK_LEN - 1);
                irc_nick[NICK_LEN-1] = '\0';
                LOG_FR("nick %s", irc_nick);
            }

            while ((ptr = strtok(NULL, " ")) != NULL){
                if (strcmp(ptr, "PRIVMSG") == 0){
                    LOG_FR("PRIVMG");
                    privmsg = 1;
                    break;
                }
            }

            if (privmsg){
                if ((ptr = strtok(NULL, ":")) != NULL && (ptr = strtok(NULL, "")) != NULL){
                    strncpy(irc_msg, ptr, MSG_LEN - 1);
                    irc_msg[MSG_LEN - 1] = '\0';
                    LOG_FR("received message %s", irc_msg);
                }
            }

            if (privmsg && strlen(irc_nick) > 0 && strlen(irc_msg) > 0){
                // irc_log_message(irc, irc_nick, irc_msg);
                LOG_FR("MSG");
                return 0;
            }
        }
    }
    return -1;
}
int irc_recv(IRC *irc, char *irc_nick, char *irc_msg){
    char tmpbuf[512];
    int rc, i;

    if ((rc = sck_recv(irc->fd, tmpbuf, sizeof(tmpbuf) - 2 )) <= 0 ){
        ERR_FR("buffer overflow");
        return -1;
    }

    tmpbuf[rc] = '\0';

    for (i = 0; i < rc; ++i ){
        switch (tmpbuf[i]){
            case '\r':
            case '\n': {
                           irc->servbuf[irc->bufptr] = '\0';
                           irc->bufptr = 0;
                           return irc_parse(irc, irc_nick, irc_msg);
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
    return -1;
}


void irc_close(IRC *irc){
    close(irc->fd);
}
