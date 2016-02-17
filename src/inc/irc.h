#ifndef __IRC_H
#define __IRC_H

#define CHAN_NUM    50
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
   char *nick;
   char *server;
   char *alias;

   int nchan;
   char chans[CHAN_NUM][CHAN_LEN];
   char servbuf[BUF_LEN];
   int bufptr;
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
    IRCMSG_UNKNOWN
} irc_msg_type_t;

int irc_connect(irc_t *irc, const char *server, const char *port);
int irc_reg(irc_t *irc, const char *nick, const char *username, const char *fullname);
int irc_join(irc_t *irc, const char *chan);
int irc_part(irc_t *irc, const char *chan, const char *reason);
int irc_msg(irc_t *irc, const char *chan, const char *msg);
irc_msg_type_t irc_recv(irc_t *irc, irc_msg_t *ircmsg);
int irc_quit(irc_t *irc, const char *reason);
void irc_close(irc_t *irc);

#endif /* __IRC_H */
