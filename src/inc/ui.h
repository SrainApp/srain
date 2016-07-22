#ifndef __UI_H
#define __UI_H

#include "srain_msg.h"
#include "srain_user_list.h"

void ui_add_chan(const char *server_name, const char *chan_name);
void ui_rm_chan(const char *server_name, const char *chan_name);

void ui_sys_msg(const char *server_name, const char *chan_name, const char *msg, SysMsgType type);
void ui_send_msg(const char *server_name, const char *chan_name, const char *msg);
void ui_recv_msg(const char *server_name, const char *chan_name, const char *nick, const char *id, const char *msg);

void ui_user_list_add(const char *server_name, const char *chan_name, const char *nick, UserType type, int notify);
void ui_user_list_rm(const char *server_name, const char *chan_name, const char *nick, const char *reason);
void ui_user_list_rename(const char *server_name, const char *chan_name, const char *old_nick, const char *new_nick, UserType type);

void ui_set_topic(const char *server_name, const char *chan_name, const char *topic);

#endif /* __UI_H */
