#ifndef __IRC_CMD_H
#define __IRC_CMD_H

/* IRC Protocol */
int irc_cmd_pong(int fd, const char *pong);
int irc_cmd_join(int fd, const char *chan);
int irc_cmd_user(int fd, const char *username, const char *hostname, const char *servername, const char *realname);
int irc_cmd_part(int fd, const char *chan, const char *reason);
int irc_cmd_nick(int fd, const char *nick);
int irc_cmd_quit(int fd, const char *reason);
int irc_cmd_topic(int fd, const char *chan, const char *topic);
int irc_cmd_action(int fd, const char *who, const char *action);
int irc_cmd_msg(int fd, const char *who, const char *msg);
int irc_cmd_whois(int fd, const char *who);
int irc_cmd_names(int fd, const char *chan);
int irc_cmd_invite(int fd, const char *nick, const char *chan);
int irc_cmd_kick(int fd, const char *nick, const char *chan, const char *reason);
int irc_cmd_mode(int fd, const char *target, const char *mode);
int irc_cmd_raw(int fd, const char *fmt, ...);

#endif /* __IRC_CMD_H */
