#ifndef __IRC_CMD_H
#define __IRC_CMD_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

/* IRC Protocol */
int sirc_cmd_pong(SircSession *sirc, const char *pong);
int sirc_cmd_join(SircSession *sirc, const char *chan, const char *passwd);
int sirc_cmd_user(SircSession *sirc, const char *username, const char *hostname, const char *servername, const char *realname);
int sirc_cmd_part(SircSession *sirc, const char *chan, const char *reason);
int sirc_cmd_nick(SircSession *sirc, const char *nick);
int sirc_cmd_quit(SircSession *sirc, const char *reason);
int sirc_cmd_topic(SircSession *sirc, const char *chan, const char *topic);
int sirc_cmd_action(SircSession *sirc, const char *who, const char *action);
int sirc_cmd_msg(SircSession *sirc, const char *who, const char *msg);
int sirc_cmd_whois(SircSession *sirc, const char *who);
int sirc_cmd_names(SircSession *sirc, const char *chan);
int sirc_cmd_invite(SircSession *sirc, const char *nick, const char *chan);
int sirc_cmd_kick(SircSession *sirc, const char *nick, const char *chan, const char *reason);
int sirc_cmd_mode(SircSession *sirc, const char *target, const char *mode);
int sirc_cmd_raw(SircSession *sirc, const char *fmt, ...);

#endif /* __IRC_CMD_H */
