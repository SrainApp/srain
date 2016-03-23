#ifndef __IRC_CORE_H
#define __IRC_CORE_H

/* IRC Protocol */
int irc_core_pong(int fd, const char *pong);
int irc_core_reg(int fd, const char *nick, const char *user_name, const char *full_name);
int irc_core_join(int fd, const char *chan);
int irc_core_part(int fd, const char *chan, const char *reason);
int irc_core_nick(int fd, const char *nick);
int irc_core_quit(int fd, const char *reason);
int irc_core_topic(int fd, const char *chan, const char *topic);
int irc_core_action(int fd, const char *who, const char *action);
int irc_core_msg(int fd, const char *who, const char *msg);
int irc_core_whois(int fd, const char *who);
int irc_core_names(int fd, const char *chan);
int irc_core_invite(int fd, const char *nick, const char *chan);
int irc_core_kick(int fd, const char *nick, const char *chan, const char *reason);
int irc_core_mode(int fd, const char *target, const char *mode);

#endif /* __IRC_CORE_H */
