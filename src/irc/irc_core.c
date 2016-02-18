/* irc_core.c
 * This file is modified from <https://github.com/Themaister/simple-irc-bot>
 */

#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "irc.h"
#include "irc_parse.h"
#include "log.h"

int irc_connect(irc_t *irc, const char *server, const char *port){
    LOG_FR("connecting to %s:%s", server, port);
    if ((irc->fd =  get_socket(server, port)) < 0){
        return -1;
    }
    irc->bufptr = 0;
    memset(irc->chans, 0, sizeof(irc->chans));

    return 0;
}

// irc_pong: For answering pong requests...
int irc_pong(irc_t *irc, const char *data){
    return sck_sendf(irc->fd, "PONG :%s\r\n", data);
}

// irc_reg: For registering upon login
int irc_reg(irc_t *irc, const char *nick, const char *username, const char *fullname){
    LOG_FR("attemping to login as %s", nick);
    irc->nick = (char *)nick;
    if (!username) username = "Srain";
    if (!fullname) fullname = "EL PSY CONGRO";

    return sck_sendf(irc->fd, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

// irc_join: For joining a chan
int irc_join(irc_t *irc, const char *chan){
    int i, empty = -1;

    LOG_FR("join %s", chan);
    for (i = 0; i < CHAN_NUM; i++){
        if (strncmp(irc->chans[i], chan, CHAN_LEN) == 0){
            ERR_FR("channels already exist");
            return -1;
        }
        if (strlen(irc->chans[i]) == 0)
            empty = (empty != -1 && empty < i) ? empty : i;
    }
    if (empty != -1 && sck_sendf(irc->fd, "JOIN %s\r\n", chan) != -1){
        strncpy(irc->chans[empty], chan, CHAN_LEN);
        return 0;
    }

    ERR_FR("channels list is full");
    return -1;
}

// irc_part: For leaving a chan
int irc_part(irc_t *irc, const char *chan, const char *reason){
    int i;

    LOG_FR("part %s reason %s", chan, reason);

    // reasion doesn't wrok TODO
    for (i = 0; i < CHAN_NUM; i++){
        if (strncmp(irc->chans[i], chan, CHAN_LEN) == 0
                && sck_sendf(irc->fd, "PART %s :%s\r\n", chan, reason) != -1){
            memset(irc->chans[i], 0, CHAN_LEN);
            return 0;
        }
    }

    ERR_FR("no such channel %s", chan);
    return -1;
}

// irc_nick: For changing your nick
int irc_nick(irc_t *irc, const char *nick){
    LOG_FR("nick %s", nick);
    return sck_sendf(irc->fd, "NICK %s\r\n", nick);
}

// irc_quit: For quitting IRC
int irc_quit(irc_t *irc, const char *reason){
    LOG_FR("reason: %s", reason);

    return sck_sendf(irc->fd, "QUIT :%s\r\n", reason);
}

// irc_quit: For closeing connection
void irc_close(irc_t *irc){
    close(irc->fd);
}

// irc_topic: For setting or removing a topic
int irc_topic(irc_t *irc, const char *chan, const char *topic){
    LOG_FR("chan %s topic %s", chan, topic);

    return sck_sendf(irc->fd, "TOPIC %s :%s\r\n", chan, topic);
}

// irc_action: For executing an action (.e.g /me is hungry)
int irc_action(irc_t *irc, const char *chan, const char *msg){
    LOG_FR("chan %s msg %s", chan, msg);

    return sck_sendf(irc->fd, "PRIVMSG %s :\001ACTION %s\001\r\n", chan, msg);
}

// irc_msg: For sending a chan message or a query
int irc_msg(irc_t *irc, const char *chan, const char *msg){
    LOG_FR("chan %s msg %s", chan, msg);

    return sck_sendf(irc->fd, "PRIVMSG %s :%s\r\n", chan, msg);
}

int irc_names(irc_t *irc, const char *chan){
    LOG_FR("names %s", chan);

    return sck_sendf(irc->fd, "NAMES %s\r\n", chan);
}

int irc_whois(irc_t *irc, const char *who){
    LOG_FR("whois %s", who);

    return sck_sendf(irc->fd, "WHOIS %s\r\n", who);
}

irc_msg_type_t irc_recv(irc_t *irc, irc_msg_t *ircmsg){
    int i;
    static int rc, tmpbuf_ptr = 0;
    static char tmpbuf[BUF_LEN];

    if (tmpbuf_ptr == 0){
        if ((rc = sck_recv(irc->fd, tmpbuf, BUF_LEN -2)) <= 0 ){
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
                           res = irc_parse(irc->servbuf, ircmsg, 0);
                           switch (res){
                               case IRCMSG_PING:
                                   irc_pong(irc, irc->servbuf);
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
