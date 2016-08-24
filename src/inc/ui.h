#ifndef __UI_H
#define __UI_H

#include "srain_msg.h"
#include "srain_user_list.h"

void ui_init(int argc, char **argv);
/* UI interface functions, used by other module */
void ui_add_chan(const char *srv_name, const char *chan_name);
void ui_rm_chan(const char *srv_name, const char *chan_name);

void ui_sys_msg(const char *srv_name, const char *chan_name, const char *msg, SysMsgType type);
void ui_send_msg(const char *srv_name, const char *chan_name, const char *msg);
void ui_recv_msg(const char *srv_name, const char *chan_name, const char *nick, const char *id, const char *msg);

void ui_user_list_add(const char *srv_name, const char *chan_name, const char *nick, UserType type);
void ui_user_list_rm(const char *srv_name, const char *chan_name, const char *nick);
void ui_user_list_rename(const char *srv_name, const char *chan_name, const char *old_nick, const char *new_nick, UserType type);

void ui_set_topic(const char *srv_name, const char *chan_name, const char *topic);

/* Synchronous functions, export them for testing */
int ui_add_chan_sync(const char *srv_name, const char *chan_name);
int ui_rm_chan_sync(const char *srv_name, const char *chan_name);

void ui_sys_msg_sync(const char *srv_name, const char *chan_name, const char *msg, SysMsgType type);
void ui_send_msg_sync(const char *srv_name, const char *chan_name, const char *msg);
void ui_recv_msg_sync(const char *srv_name, const char *chan_name, const char *nick, const char *id, const char *msg);

int ui_user_list_add_sync(const char *srv_name, const char *chan_name, const char *nick, UserType type);
int ui_user_list_rm_sync(const char *srv_name, const char *chan_name, const char *nick);
void ui_user_list_rename_sync(const char *srv_name, const char *chan_name, const char *old_nick, const char *new_nick, UserType type);

void ui_set_topic_sync(const char *srv_name, const char *chan_name, const char *topic);
#endif /* __UI_H */
