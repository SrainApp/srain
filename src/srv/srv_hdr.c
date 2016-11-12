#include "srv_hdr.h"
#include "srv_session.h"
#include "srv_test.h"

#include "log.h"

void srv_hdr_init(){
#ifdef IRC_TEST
    srv_hdr_ui_add_chat = srv_test_ui_add_chat;
    srv_hdr_ui_rm_chat = srv_test_ui_rm_chat;
    srv_hdr_ui_sys_msg = srv_test_ui_sys_msg;
    srv_hdr_ui_send_msg = srv_test_ui_send_msg;
    srv_hdr_ui_recv_msg = srv_test_ui_recv_msg;
    srv_hdr_ui_add_user = srv_test_ui_add_user;
    srv_hdr_ui_rm_user = srv_test_ui_rm_user;
    srv_hdr_ui_ren_user = srv_test_ui_ren_user;
    srv_hdr_ui_set_topic = srv_test_ui_set_topic;
#else
    srv_hdr_ui_add_chat = ui_add_chat;
    srv_hdr_ui_rm_chat = ui_rm_chat;
    srv_hdr_ui_sys_msg = ui_sys_msg;
    srv_hdr_ui_send_msg = ui_send_msg;
    srv_hdr_ui_recv_msg = ui_recv_msg;
    srv_hdr_ui_add_user = ui_add_user;
    srv_hdr_ui_rm_user = ui_rm_user;
    srv_hdr_ui_ren_user = ui_ren_user;
    srv_hdr_ui_set_topic = ui_set_topic;
#endif
}

void srv_hdr_ui_sys_msgf(SIGN_UI_CHAT_ID, SysMsgType type, SrainMsgFlag flag,
        const char *fmt, ...) {
    char buf[MSG_LEN];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    srv_hdr_ui_sys_msg(srv_name, chat_name, buf, type, flag);
}
