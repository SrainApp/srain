/**
 * @file irc_cmd.c
 * @brief Send IRC command to server
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * Copied from <https://github.com/Themaister/simple-irc-bot>
 *
 */

#define __LOG_ON

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "srv.h"

#include "irc_cmd.h"
#include "socket.h"

#include "srain.h"
#include "log.h"

// irc_cmd_pong: For answering pong requests...
int irc_cmd_pong(int fd, const char *data){
    return irc_cmd_raw(fd, "PONG :%s\r\n", data);
}

int irc_cmd_user(int fd, const char *username, const char *hostname,
        const char *servername, const char *realname){
    return irc_cmd_raw(fd, "USER %s %s %s :%s\r\n",
            username, hostname, servername, realname);
}

// irc_cmd_join: For joining a chan
int irc_cmd_join(int fd, const char *chan){
    return irc_cmd_raw(fd, "JOIN %s\r\n", chan);
}

// irc_cmd_part: For leaving a chan
int irc_cmd_part(int fd, const char *chan, const char *reason){
    return irc_cmd_raw(fd, "PART %s :%s\r\n", chan, reason);
}

// irc_cmd_nick: For changing your nick
int irc_cmd_nick(int fd, const char *nick){
    return irc_cmd_raw(fd, "NICK %s\r\n", nick);
}

// irc_cmd_quit: For quitting IRC
int irc_cmd_quit(int fd, const char *reason){
    return irc_cmd_raw(fd, "QUIT :%s\r\n", reason);
}


// irc_cmd_topic: For setting or removing a topic
int irc_cmd_topic(int fd, const char *chan, const char *topic){
    return irc_cmd_raw(fd, "TOPIC %s :%s\r\n", chan, topic);
}

// irc_cmd_action: For executing an action (.e.g /me is hungry)
int irc_cmd_action(int fd, const char *chan, const char *msg){
    return irc_cmd_raw(fd, "PRIVMSG %s :\001ACTION %s\001\r\n", chan, msg);
}

// irc_cmd_msg: For sending a chan message or a query
int irc_cmd_msg(int fd, const char *chan, const char *msg){
    return irc_cmd_raw(fd, "PRIVMSG %s :%s\r\n", chan, msg);
}

int irc_cmd_names(int fd, const char *chan){
    return irc_cmd_raw(fd, "NAMES %s\r\n", chan);
}

int irc_cmd_whois(int fd, const char *who){
    return irc_cmd_raw(fd, "WHOIS %s\r\n", who);
}

int irc_cmd_invite(int fd, const char *nick, const char *chan){
    return irc_cmd_raw(fd, "INVITE %s %s\r\n", nick, chan);
}

int irc_cmd_kick(int fd, const char *nick, const char *chan,
        const char *reason){
    return irc_cmd_raw(fd, "KICK %s %s :%s\r\n", chan, nick, reason);
}

int irc_cmd_mode(int fd, const char *target, const char *mode){
    return irc_cmd_raw(fd, "MODE %s %s\r\n", target, mode);
}

int irc_cmd_raw(int fd, const char *fmt, ...){
    char buf[BUF_LEN];
    int len = 0;
    va_list args;

    LOG_FR("fd: %d, fmt: %s", fd, fmt);
    if (strlen(fmt) != 0){
        va_start(args, fmt);
        len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
    }
    
    if (len > 512){
        WARN_FR("Raw command too long");
        len = 512;
    }

    int ret = sck_send(fd, buf, len);
    if (ret == SRN_ERR){
        return SRN_ERR;
    }
    // TODO send it totally
    // if (ret < len) ...

    return SRN_OK;
}
