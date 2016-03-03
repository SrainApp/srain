#ifndef __IRC_CORE_H
#define __IRC_CORE_H

/* IRC Protocol */
int irc_pong(int fd, const char *pong);
int irc_reg(int fd, const char *nick, const char *user_name, const char *full_name);
int irc_join(int fd, const char *chan);
int irc_part(int fd, const char *chan, const char *reason);
int irc_nick(int fd, const char *nick);
int irc_quit(int fd, const char *reason);
int irc_topic(int fd, const char *chan, const char *topic);
int irc_action(int fd, const char *who, const char *action);
int irc_msg(int fd, const char *who, const char *msg);
int irc_whois(int fd, const char *who);
int irc_names(int fd, const char *chan);

#endif /* __IRC_CORE_H */
