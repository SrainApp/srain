#ifndef __LIBIRCCLIENT_EX_H
#define __LIBIRCCLIENT_EX_H

#include <libircclient.h>

#define NORFC1459_RPL_STATSDLINE    250
#define NORFC1459_RPL_LOCALUSERS    265
#define NORFC1459_RPL_GLOBALUSERS   266
#define NORFC1459_RPL_CHANNEL_URL   328
#define NORFC1459_RPL_WHOWAS_TIME 	330
#define NORFC1459_RPL_WHOISHOST     378
#define NORFC1459_RPL_WHOISSECURE   671

int irc_cmd_who (irc_session_t *session, const char *nick);

#endif /* __LIBIRCCLIENT_EX_H */
