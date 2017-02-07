#ifndef __IRC_CMD_H
#define __IRC_CMD_H

#include "irc.h"

/* IRC Protocol */
int irc_cmd_pong(SircSession *sirc, const char *pong);
int irc_cmd_join(SircSession *sirc, const char *chan);
int irc_cmd_user(SircSession *sirc, const char *username, const char *hostname, const char *servername, const char *realname);
int irc_cmd_part(SircSession *sirc, const char *chan, const char *reason);
int irc_cmd_nick(SircSession *sirc, const char *nick);
int irc_cmd_quit(SircSession *sirc, const char *reason);
int irc_cmd_topic(SircSession *sirc, const char *chan, const char *topic);
int irc_cmd_action(SircSession *sirc, const char *who, const char *action);
int irc_cmd_msg(SircSession *sirc, const char *who, const char *msg);
int irc_cmd_whois(SircSession *sirc, const char *who);
int irc_cmd_names(SircSession *sirc, const char *chan);
int irc_cmd_invite(SircSession *sirc, const char *nick, const char *chan);
int irc_cmd_kick(SircSession *sirc, const char *nick, const char *chan, const char *reason);
int irc_cmd_mode(SircSession *sirc, const char *target, const char *mode);
int irc_cmd_raw(SircSession *sirc, const char *fmt, ...);

#endif /* __IRC_CMD_H */
