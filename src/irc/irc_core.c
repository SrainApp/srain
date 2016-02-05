/* irc_core.c
 * This file is modified from <https://github.com/Themaister/simple-irc-bot>
 */

#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "irc.h"
#include "log.h"

// irc_pong: For answering pong requests...
int irc_pong(int fd, const char *data){
   return sck_sendf(fd, "PONG :%s\r\n", data);
}

// irc_reg: For registering upon login
int irc_reg(int fd, const char *nick, const char *username, const char *fullname){
   return sck_sendf(fd, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

// irc_join: For joining a chan
int irc_join(int fd, const char *chan){
   return sck_sendf(fd, "JOIN %s\r\n", chan);
}

// irc_part: For leaving a chan
int irc_part(int fd, const char *reason){
   return sck_sendf(fd, "PART %s\r\n", reason);
}

// irc_nick: For changing your nick
int irc_nick(int fd, const char *nick){
   return sck_sendf(fd, "NICK %s\r\n", nick);
}

// irc_quit: For quitting IRC
int irc_quit(int fd, const char *reason){
   return sck_sendf(fd, "QUIT :%s\r\n", reason);
}

// irc_topic: For setting or removing a topic
int irc_topic(int fd, const char *chan, const char *topic){
   return sck_sendf(fd, "TOPIC %s :%s\r\n", chan, topic);
}

// irc_action: For executing an action (.e.g /me is hungry)
int irc_action(int fd, const char *chan, const char *msg){
   return sck_sendf(fd, "PRIVMSG %s :\001ACTION %s\001\r\n", chan, msg);
}

// irc_msg: For sending a chan message or a query
int irc_msg(int fd, const char *chan, const char *msg){
   return sck_sendf(fd, "PRIVMSG %s :%s\r\n", chan, msg);
}

int irc_names(int fd, const char *chan){
   return sck_sendf(fd, "NAMES %s\r\n", chan);
}

int irc_whois(int fd, const char *who){
   return sck_sendf(fd, "WHOIS %s\r\n", who);
}
