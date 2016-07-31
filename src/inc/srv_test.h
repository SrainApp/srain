#ifndef __SRV_TEST
#define __SRV_TEST

#include "ui.h"

void srv_test();
void srv_test_ui_add_chan(const char *server_name, const char *chan_name);
void srv_test_ui_rm_chan(const char *server_name, const char *chan_name);
void srv_test_ui_sys_msg(const char *server_name, const char *chan_name, const char *msg, SysMsgType type);
void srv_test_ui_send_msg(const char *server_name, const char *chan_name, const char *msg);
void srv_test_ui_recv_msg(const char *server_name, const char *chan_name, const char *nick, const char *id, const char *msg);
void srv_test_ui_user_list_add(const char *server_name, const char *chan_name, const char *nick, UserType type);
void srv_test_ui_user_list_rm(const char *server_name, const char *chan_name, const char *nick);
void srv_test_ui_user_list_rm_all(const char *server_name, const char *nick, const char *reason);
void srv_test_ui_user_list_rename(const char *server_name, const char *old_nick, const char *new_nick, UserType type, const char *msg);
void srv_test_ui_set_topic(const char *server_name, const char *chan_name, const char *topic);

#endif /* __SRV_TEST */
