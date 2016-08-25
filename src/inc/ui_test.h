#ifndef __UI_TEST_H
#define __UI_TEST_H

#include "srv.h"

DECLARE_SRVJoinFunc(ui_test_srv_join);
DECLARE_SRVPartFunc(ui_test_srv_part);
DECLARE_SRVSendFunc(ui_test_srv_send);
DECLARE_SRVCmdFunc(ui_test_srv_cmd);

void ui_test();

#endif /* __UI_TEST_H */
