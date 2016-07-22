#include "srv_hdr.h"
#include "srv_test.h"

#include "log.h"

void srv_hdr_init(){
#ifdef IRC_TEST
    srv_hdr_ui_add_chan = srv_test_ui_add_chan;
    srv_hdr_ui_rm_chan = srv_test_ui_rm_chan;
    srv_hdr_ui_sys_msg = srv_test_ui_sys_msg;
    srv_hdr_ui_send_msg = srv_test_ui_send_msg;
    srv_hdr_ui_recv_msg = srv_test_ui_recv_msg;
    srv_hdr_ui_user_list_add = srv_test_ui_user_list_add;
    srv_hdr_ui_user_list_rm = srv_test_ui_user_list_rm;
    srv_hdr_ui_user_list_rename = srv_test_ui_user_list_rename;
    srv_hdr_ui_set_topic = srv_test_ui_set_topic;
#else
    srv_hdr_ui_add_chan = ui_add_chan;
    srv_hdr_ui_rm_chan = ui_rm_chan;
    srv_hdr_ui_sys_msg = ui_sys_msg;
    srv_hdr_ui_send_msg = ui_send_msg;
    srv_hdr_ui_recv_msg = ui_recv_msg;
    srv_hdr_ui_user_list_add = ui_user_list_add;
    srv_hdr_ui_user_list_rm = ui_user_list_rm;
    srv_hdr_ui_user_list_rename = ui_user_list_rename;
    srv_hdr_ui_set_topic = ui_set_topic;
#endif
}
