#ifndef __IRC_H
#define __IRC_H

#define CHAN_NUM    50
#define CHAN_LEN    256
#define BUF_LEN     512
#define NICK_LEN    128
#define MSG_LEN     512

typedef struct {
   int fd;
   char *nick;
   unsigned int nchan;
   char chans[CHAN_NUM][CHAN_LEN];
   char servbuf[BUF_LEN];
   int bufptr;
} IRC; 

int irc_connect(IRC *irc, const char* server, const char* port);
int irc_login(IRC *irc, const char* nick);
int irc_join_chan(IRC *irc, const char* chan);
int irc_leave_chan(IRC *irc, const char *chan);
int irc_send(IRC *irc, const char *chan, const char *msg);
int irc_recv(IRC *irc, char *nick, char *chan, char *cmd, char *msg);
void irc_close(IRC *irc);

#endif /* __IRC_H */
