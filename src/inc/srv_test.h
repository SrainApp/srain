#ifndef __SRV_TEST
#define __SRV_TEST

#include "ui.h"

void srv_test();

DECLARE_UIAddChatFunc(srv_test_ui_add_chan);
DECLARE_UIRmChatFunc(srv_test_ui_rm_chat);
DECLARE_UISysMsgFunc(srv_test_ui_sys_msg);
DECLARE_UISendMsgFunc(srv_test_ui_send_msg);
DECLARE_UIRecvMsgFunc(srv_test_ui_recv_msg);
DECLARE_UIUserListAddFunc(srv_test_ui_user_list_add);
DECLARE_UIUserListRmFunc(srv_test_ui_user_list_rm);
DECLARE_UIUserListRenameFunc(srv_test_ui_user_list_rename);
DECLARE_UISetTopicFunc(srv_test_ui_set_topic);

#endif /* __SRV_TEST */
