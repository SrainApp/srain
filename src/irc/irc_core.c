/**
 * @file irc_core.c
 * @brief simple implement of irc client protocol
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * simple implement of irc client protocol, the original edition is
 * copide from * <https://github.com/Themaister/simple-irc-bot>
 *
 * only called by irc.c
 *
 */

#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "log.h"

// irc_core_pong: For answering pong requests...
int irc_core_pong(int fd, const char *data){
    return sck_sendf(fd, "PONG :%s\r\n", data);
}

// irc_core_reg: For registering upon login
int irc_core_reg(int fd, const char *nick, const char *username,
        const char *fullname){
    LOG_FR("attemping to login as %s", nick);

    return sck_sendf(fd, "NICK %s\r\nUSER %s localhost 0 :%s\r\n",
            nick, username, fullname);
}

// irc_core_join: For joining a chan
int irc_core_join(int fd, const char *chan){
    LOG_FR("join %s", chan);

    return sck_sendf(fd, "JOIN %s\r\n", chan);
}

// irc_core_part: For leaving a chan
int irc_core_part(int fd, const char *chan, const char *reason){
    LOG_FR("part %s reason %s", chan, reason);

    // reasion doesn't wrok TODO
    return sck_sendf(fd, "PART %s :%s\r\n", chan, reason);
}

// irc_core_nick: For changing your nick
int irc_core_nick(int fd, const char *nick){
    LOG_FR("nick %s", nick);
    return sck_sendf(fd, "NICK %s\r\n", nick);
}

// irc_core_quit: For quitting IRC
int irc_core_quit(int fd, const char *reason){
    LOG_FR("reason: %s", reason);

    return sck_sendf(fd, "QUIT :%s\r\n", reason);
}


// irc_core_topic: For setting or removing a topic
int irc_core_topic(int fd, const char *chan, const char *topic){
    LOG_FR("chan %s topic %s", chan, topic);

    return sck_sendf(fd, "TOPIC %s :%s\r\n", chan, topic);
}

// irc_core_action: For executing an action (.e.g /me is hungry)
int irc_core_action(int fd, const char *chan, const char *msg){
    LOG_FR("chan %s msg %s", chan, msg);

    return sck_sendf(fd, "PRIVMSG %s :\001ACTION %s\001\r\n", chan, msg);
}

// irc_core_msg: For sending a chan message or a query
int irc_core_msg(int fd, const char *chan, const char *msg){
    LOG_FR("chan %s msg %s", chan, msg);

    return sck_sendf(fd, "PRIVMSG %s :%s\r\n", chan, msg);
}

int irc_core_names(int fd, const char *chan){
    LOG_FR("names %s", chan);

    return sck_sendf(fd, "NAMES %s\r\n", chan);
}

int irc_core_whois(int fd, const char *who){
    LOG_FR("whois %s", who);

    return sck_sendf(fd, "WHOIS %s\r\n", who);
}

int irc_core_invite(int fd, const char *nick, const char *chan){
    LOG_FR("invite %s %s", nick, chan);

    return sck_sendf(fd, "INVITE %s %s\r\n", nick, chan);
}

int irc_core_kick(int fd, const char *nick, const char *chan,
        const char *reason){
    LOG_FR("nick %s chan %s reason %s", nick, chan, reason);

    return sck_sendf(fd, "KICK %s %s :%s\r\n", chan, nick, reason);
}

int irc_core_mode(int fd, const char *target, const char *mode){
    LOG_FR("target %s mode %s", target, mode);

    return sck_sendf(fd, "MODE %s %s\r\n", target, mode);
}
