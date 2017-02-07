#ifndef __IRC_PARSE_H
#define __IRC_PARSE_H

#define SIRC_CMD_LEN        64      // unconfirm
#define SIRC_PARAM_COUNT    64      // unconfirm
#define SIRC_PARAM_LEN      64      // unconfirm
#define SIRC_MSG_LEN        512     // unconfirm

typedef struct {
    char prefix[64]; // servername or nick!user@host
    char *nick, *user, *host;

    char cmd[SIRC_CMD_LEN];
    int nparam;
    char params[SIRC_PARAM_COUNT][SIRC_PARAM_LEN];  // middle
    char msg[SIRC_MSG_LEN];  // trailing

    void *ctx;   // irc server this message belongs to
} SircMessage;

typedef enum {
    SIRC_MSG_PING,
    SIRC_MSG_NOTICE,
    SIRC_MSG_ERROR,

    SIRC_MSG_MESSAGE,
    SIRC_MSG_UNKNOWN,
    SIRC_MSG_SCKERR
} SircMessageType;

SircMessageType irc_parse(char *line, SircMessage *imsg);

#endif /* __IFC_PARSE_H */
