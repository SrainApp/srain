#ifndef __IRC_PARSE_H
#define __IRC_PARSE_H

#include "irc.h"
#include "irc_magic.h"

typedef struct {
    char prefix[64]; // servername or nick!user@host
    char *nick, *user, *host;

    char command[COMMAND_LEN];
    int nparam;
    char param[PARAM_COUNT][PARAM_LEN];  // middle
    char message[SIRC_MSG_LEN];  // trailing

    void *ctx;   // irc server this message belongs to
} IRCMsg;

typedef enum {
    IRCMSG_PING,
    IRCMSG_NOTICE,
    IRCMSG_ERROR,

    IRCMSG_MSG,
    IRCMSG_UNKNOWN,
    IRCMSG_SCKERR
} IRCMsgType;

IRCMsgType irc_parse(char *line, IRCMsg *ircmsg);

#endif /* __IFC_PARSE_H */
