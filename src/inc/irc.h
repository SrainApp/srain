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
#define PRARM_COUNT 64  // unconfirm
#define PRARM_LEN   64  // unconfirm

typedef struct {
   int fd;
   char *nick;
   unsigned int nchan;
   char chans[CHAN_NUM][CHAN_LEN];
   char servbuf[BUF_LEN];
   int bufptr;
} IRC;

typedef struct {
    char servername[SERVER_LEN];
    char nick[NICK_LEN], user[USER_LEN], host[HOST_LEN];

    char command[COMMAND_LEN];
    int nparam;
    char param[PRARM_COUNT][PRARM_LEN];  // middle
    char message[MSG_LEN];  // trailing
} IRCMsg;

typedef enum {
    IRCMSG_PING,
    IRCMSG_NOTICE,
    IRCMSG_ERROR,
    IRCMSG_MSG,
    IRCMSG_SERVER,
    IRCMSG_UNKNOWN
} IRCMsgType;

int irc_connect(IRC *irc, const char* server, const char* port);
int irc_login(IRC *irc, const char* nick);
int irc_join_chan(IRC *irc, const char* chan);
int irc_leave_chan(IRC *irc, const char *chan);
int irc_send(IRC *irc, const char *chan, const char *msg);
IRCMsgType irc_recv(IRC *irc, IRCMsg *ircmsg);
void irc_close(IRC *irc);

#endif /* __IRC_H */
