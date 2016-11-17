#ifndef __SRV_H
#define __SRV_H

#include "interfaces.h"

void srv_init();
void srv_finalize();

/* SRV interface functions, used by other module */
int srv_connect(SIGN_SRV_CONNECT);
int srv_quit(SIGN_SRV_QUIT);
int srv_cmd  (SIGN_SRV_CMD);
int srv_query(SIGN_SRV_QUERY);
int srv_unquery(SIGN_SRV_UNQUERY);
int srv_join (SIGN_SRV_JOIN);
int srv_part (SIGN_SRV_PART);
int srv_send (SIGN_SRV_SEND);
int srv_kick(SIGN_SRV_KICK);
int srv_whois(SIGN_SRV_WHOIS);
int srv_invite(SIGN_SRV_INVITE);

#endif /* __SRV_H */
