#ifndef __IRC_H
#define __IRC_H

#include <glib.h>
#include "irc_magic.h"

#define BUF_LEN     512

typedef struct {
    // TODO: remove alias; let IRCServer handle nick
    int fd;
    int bufptr;
    char nick[NICK_LEN];
    char server[256];
    char alias[BUF_LEN];
    char servbuf[BUF_LEN];
    GList *chans;
} IRC;

typedef struct {
    char servername[SERVER_LEN];
    char nick[NICK_LEN], user[USER_LEN], host[HOST_LEN];

    char command[COMMAND_LEN];
    int nparam;
    char param[PARAM_COUNT][PARAM_LEN];  // middle
    char message[MSG_LEN];  // trailing

    void *server;   // irc server this message belongs to
} IRCMsg;

typedef enum {
    IRCMSG_PING,
    IRCMSG_NOTICE,
    IRCMSG_ERROR,

    IRCMSG_MSG,
    IRCMSG_UNKNOWN,
    IRCMSG_SCKERR
} IRCMsgType;

typedef enum {
    SEND_MSG,
    SEND_ME
} send_type_t;

int irc_connect(IRC *irc, const char *server, const char *port);
int irc_login(IRC *irc, const char *nick);
void irc_close(IRC *irc);
int irc_quit_req(IRC *irc, const char *reason);

int irc_join_req(IRC *irc, const char *chan);
int irc_part_req(IRC *irc, const char *chan, const char *reason);
void irc_join_ack(IRC *irc, const char *chan);
void irc_part_ack(IRC *irc, const char *chan);

int irc_nick_req(IRC *irc, const char *nick);
void irc_nick_ack(IRC *irc, const char *nick);

int irc_whois(IRC *irc, const char *nick);
int irc_send(IRC *irc, const char *chan, const char *msg, int is_me);
int irc_invite(IRC *irc, const char *nick, const char *chan);
int irc_kick(IRC *irc, const char *nick, const char *chan, const char *reason);
IRCMsgType irc_recv(IRC *irc, IRCMsg *ircmsg);
int irc_mode(IRC *irc, const char *target, const char *mode);

#endif /* __IRC_H */
