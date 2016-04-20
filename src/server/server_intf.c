/**
 * @file server_intf.c
 * @brief server module interface
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-20
 */

#include <string.h>
#include <glib.h>
#include "server.h"
#include "server_cmd.h"
#include "srain_chan.h"
#include "srain_app.h"

void server_intf_ui_join(IRCServer *server, const char *chan_name){
}

void server_intf_ui_part(IRCServer *server, const char *chan_name){
}

void server_intf_ui_sys_msg (IRCServer *server, const char *target,
        const char *msg, SysMsgType type){
}

void server_intf_ui_send_msg(IRCServer *server, const char *target, const char *msg){
}

void server_intf_ui_recv_msg(IRCServer *server, const char *target,
        const char *nick, const char *id, const char *msg){
}

void server_intf_ui_user_join(IRCServer *server, const char *chan_name,
        const char *nick, IRCUserType type, int notify){
}

void server_intf_ui_user_part(IRCServer *server, const char *chan_name,
        const char *nick, const char *reason){
}

void server_intf_ui_set_topic(IRCServer *server, const char *target, const char *topic){
}
