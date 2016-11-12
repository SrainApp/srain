#ifndef __UI_TEST_H
#define __UI_TEST_H

#include "srv.h"

int ui_test_srv_join(SIGN_SRV_JOIN);
int ui_test_srv_part(SIGN_SRV_PART);
int ui_test_srv_send(SIGN_SRV_SEND);
int ui_test_srv_cmd(SIGN_SRV_CMD);

void ui_test();

#endif /* __UI_TEST_H */
