#ifndef __SERVER_INTF_H
#define __SERVER_INTF_H

#include "server.h"

void server_intf_ui_add_chan(IRCServer *srv, const char *chan_name);
void server_intf_ui_rm_chan(IRCServer *srv, const char *chan_name);
void server_intf_ui_sys_msg (IRCServer *srv, const char *chan_name, const char *msg, SysMsgType type);
void server_intf_ui_send_msg(IRCServer *srv, const char *chan_name, const char *msg);
void server_intf_ui_recv_msg(IRCServer *srv, const char *chan_name, const char *nick, const char *id, const char *msg);
void server_intf_ui_user_join(IRCServer *srv, const char *chan_name, const char *nick, IRCUserType type, int notify);
void server_intf_ui_user_part(IRCServer *srv, const char *chan_name, const char *nick, const char *reason);
void server_intf_ui_set_topic(IRCServer *srv, const char *chan_name, const char *topic);

#endif /* __SERVER_INTF_H */
