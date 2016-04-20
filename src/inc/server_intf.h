#ifndef __SERVER_INTF_H
#define __SERVER_INTF_H

#include "server.h"

void server_intf_ui_join(IRCServer *server, const char *chan_name);
void server_intf_ui_part(IRCServer *server, const char *chan_name);
void server_intf_ui_sys_msg (IRCServer *server, const char *target, const char *msg, SysMsgType type);
void server_intf_ui_send_msg(IRCServer *server, const char *target, const char *msg);
void server_intf_ui_recv_msg(IRCServer *server, const char *target, const char *nick, const char *id, const char *msg);
void server_intf_ui_user_join(IRCServer *server, const char *chan_name, const char *nick, IRCUserType type, int notify);
void server_intf_ui_user_part(IRCServer *server, const char *chan_name, const char *nick, const char *reason);
void server_intf_ui_set_topic(IRCServer *server, const char *target, const char *topic);

#endif /* __SERVER_INTF_H */
