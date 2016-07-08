#ifndef __SERVER_INTF_H
#define __SERVER_INTF_H

#include "server.h"

void server_intf_ui_add_chan(IRCServer *srv, const char *chan_name);
void server_intf_ui_rm_chan(IRCServer *srv, const char *chan_name);
void server_intf_ui_rm_all_chan(IRCServer *srv);
void server_intf_ui_sys_msg (IRCServer *srv, const char *chan_name, const char *msg, SysMsgType type);
void server_intf_ui_sys_msgf(IRCServer *srv, const char *chan_name, SysMsgType type, const char *fmt, ...);
void server_intf_ui_sys_msgf_bcst(IRCServer *srv, SysMsgType type, const char *fmt, ...);
void server_intf_ui_send_msg(IRCServer *srv, const char *chan_name, const char *msg);
void server_intf_ui_recv_msg(IRCServer *srv, const char *chan_name, const char *nick, const char *id, const char *msg);
void server_intf_ui_user_list_add(IRCServer *srv, const char *chan_name, const char *nick, UserType type, int notify);
void server_intf_ui_user_list_rm(IRCServer *srv, const char *chan_name, const char *nick, const char *reason);
void server_intf_ui_user_list_rm_bcst(IRCServer *srv, const char *nick, const char *reason);
void server_intf_ui_user_list_rename(IRCServer *srv, const char *chan_name, const char *old_nick, const char *new_nick);
void server_intf_ui_user_list_rename_bcst(IRCServer *srv, const char *old_nick, const char *new_nick);
void server_intf_ui_set_topic(IRCServer *srv, const char *chan_name, const char *topic);

#endif /* __SERVER_INTF_H */
