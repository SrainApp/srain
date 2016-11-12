#ifndef __SRV_TEST
#define __SRV_TEST

#include "ui.h"

void srv_test();

void srv_test_ui_add_chat  (SIGN_UI_ADD_CHAT);
void srv_test_ui_rm_chat   (SIGN_UI_RM_CHAT);
void srv_test_ui_sys_msg   (SIGN_UI_SYS_MSG);
void srv_test_ui_send_msg  (SIGN_UI_SEND_MSG);
void srv_test_ui_recv_msg  (SIGN_UI_RECV_MSG);
void srv_test_ui_add_user  (SIGN_UI_ADD_USER);
void srv_test_ui_rm_user   (SIGN_UI_RM_USER);
void srv_test_ui_ren_user  (SIGN_UI_REN_USER);
void srv_test_ui_set_topic (SIGN_UI_SET_TOPIC);

#endif /* __SRV_TEST */
