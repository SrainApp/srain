#ifndef __SIRC_PARSE_H
#define __SIRC_PARSE_H

#define SIRC_CMD_LEN        64      // unconfirm
#define SIRC_PARAM_COUNT    64      // unconfirm
#define SIRC_PARAM_LEN      64      // unconfirm
#define SIRC_MSG_LEN        512     // unconfirm

typedef enum {
    /* Standard IRC messages */
    SIRC_MSG_MESSAGE, // Unused
} SircMessageType;

typedef struct {
    const char *prefix; // servername or nick!user@host
    const char *nick, *user, *host;

    const char *cmd;
    int nparam;
    const char *params[SIRC_PARAM_COUNT];  // middle
    const char *msg;  // trailing
    SircMessageType type;

    void *ctx;   // irc server this message belongs to
} SircMessage;

int sirc_parse(char *line, SircMessage *imsg);

#endif /* __SIRC_PARSE_H */
