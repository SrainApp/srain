/**
 * @file irc.c
 * @brief IRC module interface
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-02
 *
 * IRC module interface
 * IRC varible shouldn't be modified without the following
 * functions
 *
 */

#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "meta.h"
#include "irc_magic.h"
#include "irc_core.h"
#include "irc_parse.h"
#include "log.h"


int irc_connect(IRC *irc, const char *server, const char *port){
    LOG_FR("connecting to %s:%s", server, port);
    
    memset(irc, 0, sizeof(IRC));
    strncpy(irc->alias, server, CHAN_LEN);
    strncpy(irc->server, server, sizeof(irc->server) - 1);

    if ((irc->fd = get_socket(server, port)) < 0){
        ERR_FR("can not connect to %s:%s", server, port);
        return -1;
    }

    LOG_FR("connected");
    irc->chans = g_list_append(irc->chans, META_SERVER);
    return 0;
}

int irc_login(IRC *irc, const char *nick){
    LOG_FR("attempt to login as %s", nick);

    strncpy(irc->nick, nick, NICK_LEN);

    return irc_core_reg(irc->fd, nick, META_NAME_VERSION, "EL-PSY-CONGRO");
}

// irc_quit: For closeing connection
void irc_close(IRC *irc){
    close(irc->fd);
}

int irc_quit_req(IRC *irc, const char *reason){
    return irc_core_quit(irc->fd, reason);
}

int irc_join_req(IRC *irc, const char *chan){
    return irc_core_join(irc->fd, chan);
}

int irc_part_req(IRC *irc, const char *chan, const char *reason){
    return irc_core_part(irc->fd, chan, reason);
}

int irc_nick_req(IRC *irc, const char *nick){
    return irc_core_nick(irc->fd, nick);
}

void irc_nick_ack(IRC *irc, const char *nick){
    strncpy(irc->nick, nick, NICK_LEN);
}

int irc_send(IRC *irc, const char *chan, const char *msg, int is_me){
    if (is_me){
        return irc_core_action(irc->fd, chan, msg);
    } else {
        return irc_core_msg(irc->fd, chan, msg);
    }
}

int irc_whois(IRC *irc, const char *nick){
    return irc_core_whois(irc->fd, nick);
}

int irc_invite(IRC *irc, const char *nick, const char *chan){
    return irc_core_invite(irc->fd, nick, chan);
}

int irc_kick(IRC *irc, const char *nick, const char *chan, const char *reason){
    return irc_core_kick(irc->fd, nick, chan, reason);
}

int irc_mode(IRC *irc, const char *target, const char *mode){
    return irc_core_mode(irc->fd, target, mode);
}

/************************************************************
 * NOTE: This function works in a number of different threads
 * non-thread-local static varible is NOT allowed.
 ************************************************************/
IRCMsgType irc_recv(IRC *irc, IRCMsg *ircmsg){
    int i;
    // gnu11 does NOT support `thread_local`, just use extension:
    static __thread int rc;
    static __thread int tmpbuf_ptr = 0;
    static __thread char tmpbuf[BUF_LEN];

    if (tmpbuf_ptr == 0){
        if ((rc = sck_recv(irc->fd, tmpbuf, BUF_LEN -2)) <= 0 ){
            ERR_FR("socket error, connection close");
            irc_close(irc);
            return IRCMSG_SCKERR;
        }
        tmpbuf[rc] = '\0';
        // LOG("{\n%s}\n", tmpbuf);
    }
    // LOG_FR("tmpbuf_ptr = %d, rc = %d", tmpbuf_ptr, rc);
    IRCMsgType res = IRCMSG_UNKNOWN;
    for (i = tmpbuf_ptr; i < rc; i++){
        switch (tmpbuf[i]){
            /* a respone may include one or more \r\n */
            case '\r': break;
            case '\n': {
                           irc->servbuf[irc->bufptr] = '\0';
                           irc->bufptr = 0;
                           res = irc_parse(irc->servbuf, ircmsg);
                           switch (res){
                               case IRCMSG_PING:
                                   irc_core_pong(irc->fd, irc->servbuf);
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
