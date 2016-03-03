#ifndef __IRC_H
#define __IRC_H

#include <glib.h>

#define BUF_LEN     512

#define SERVER_LEN  64  // unconfirm
#define HOST_LEN    64
#define CHAN_LEN    200
#define NICK_LEN    128 // unconfirm
#define USER_LEN    128 // unconfirm
#define COMMAND_LEN 64  // unconfirm
#define MSG_LEN     512
#define PARAM_COUNT 64  // unconfirm
#define PARAM_LEN   64  // unconfirm

typedef struct {
    int fd;
    char nick[NICK_LEN];
    char server[256];
    char alias[CHAN_LEN];
    int bufptr;
    char servbuf[BUF_LEN];

    GList *chans;
} irc_t;

typedef struct {
    char servername[SERVER_LEN];
    char nick[NICK_LEN], user[USER_LEN], host[HOST_LEN];

    char command[COMMAND_LEN];
    int nparam;
    char param[PARAM_COUNT][PARAM_LEN];  // middle
    char message[MSG_LEN];  // trailing
} irc_msg_t;

typedef enum {
    IRCMSG_PING,
    IRCMSG_NOTICE,
    IRCMSG_ERROR,

    IRCMSG_MSG,
    IRCMSG_UNKNOWN,
    IRCMSG_SCKERR
} irc_msg_type_t;

typedef enum {
    SEND_MSG,
    SEND_ME
} send_type_t;

int irc_connect(irc_t *irc, const char *server, const char *port);
int irc_login(irc_t *irc, const char *nick);
void irc_close(irc_t *irc);
void irc_quit_req(irc_t *irc, const char *reason);

int irc_join_req(irc_t *irc, const char *chan);
int irc_part_req(irc_t *irc, const char *chan, const char *reason);
void irc_join_ack(irc_t *irc, const char *chan);
void irc_part_ack(irc_t *irc, const char *chan, const char *reason);

int irc_nick_req(irc_t *irc, const char *nick);
void irc_nick_ack(irc_t *irc, const char *nick);

int irc_send(irc_t *irc, const char *chan, const char *msg, int is_me);
irc_msg_type_t irc_recv(irc_t *irc, irc_msg_t *ircmsg);

#endif /* __IRC_H */
