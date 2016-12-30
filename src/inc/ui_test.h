#ifndef __UI_TEST_H
#define __UI_TEST_H

#include "srv.h"

int ui_test_srv_connect(SIGN_SRV_CONNECT);
int ui_test_srv_query(SIGN_SRV_QUERY);
int ui_test_srv_unquery(SIGN_SRV_UNQUERY);
int ui_test_srv_join(SIGN_SRV_JOIN);
int ui_test_srv_part(SIGN_SRV_PART);
int ui_test_srv_quit(SIGN_SRV_QUIT);
int ui_test_srv_send(SIGN_SRV_SEND);
int ui_test_srv_cmd(SIGN_SRV_CMD);
int ui_test_srv_kick(SIGN_SRV_KICK);
int ui_test_srv_whois(SIGN_SRV_WHOIS);
int ui_test_srv_invite(SIGN_SRV_INVITE);

void ui_test();

#endif /* __UI_TEST_H */
