#ifndef __UI_INFT_H
#define __UI_INFT_H

#include <server.h>

void ui_intf_join(IRCServer *server, const char *chan_name);
void ui_intf_part(IRCServer *server, const char *chan_name);
void ui_intf_sys_msg (IRCServer *server, const char *target, const char *msg, SysMsgType type);
void ui_intf_send_msg(IRCServer *server, const char *target, const char *msg);
void ui_intf_recv_msg(IRCServer *server, const char *target, const char *nick, const char *id, const char *msg);
void ui_intf_user_join(IRCServer *server, const char *chan_name, const char *nick, IRCUserType type, int notify);
void ui_intf_user_part(IRCServer *server, const char *chan_name, const char *nick, const char *reason);
void ui_intf_set_topic(IRCServer *server, const char *target, const char *topic);

#endif /* __UI_INFT_H */
