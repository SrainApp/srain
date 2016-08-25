#ifndef __SRV_TEST
#define __SRV_TEST

#include "ui.h"

void srv_test();

DECLARE_UIAddChatFunc(srv_test_ui_add_chan);
DECLARE_UIRmChatFunc(srv_test_ui_rm_chat);
DECLARE_UISysMsgFunc(srv_test_ui_sys_msg);
DECLARE_UISendMsgFunc(srv_test_ui_send_msg);
DECLARE_UIRecvMsgFunc(srv_test_ui_recv_msg);
DECLARE_UIAddUserFunc(srv_test_ui_add_user);
DECLARE_UIRmUserFunc(srv_test_ui_rm_user);
DECLARE_UIRenUserFunc(srv_test_ui_ren_user);
DECLARE_UISetTopicFunc(srv_test_ui_set_topic);

#endif /* __SRV_TEST */
