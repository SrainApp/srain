#ifndef __SRV_HDR_H
#define __SRV_HDR_H

#include "ui.h"

typedef void* (*UIAddChanFunc) (const char *server_name, const char *chan_name);
typedef void (*UIRmChanFunc) (const char *server_name, const char *chan_name);
typedef void (*UISysMsgFunc) (const char *server_name, const char *chan_name, const char *msg, SysMsgType type);
typedef void (*UISendMsgFunc) (const char *server_name, const char *chan_name, const char *msg);
typedef void (*UIRecvMsgFunc) (const char *server_name, const char *chan_name, const char *nick, const char *id, const char *msg);
typedef int (*UIUserListAddFunc) (const char *server_name, const char *chan_name, const char *nick, UserType type);
typedef int (*UIUserListRmFunc) (const char *server_name, const char *chan_name, const char *nick, const char *reason);
typedef int (*UIUserListRenameFunc) (const char *server_name, const char *chan_name, const char *old_nick, const char *new_nick, UserType type);
typedef void (*UISetTopicFunc) (const char *server_name, const char *chan_name, const char *topic);

void srv_hdr_init();
void srv_hdr_ui_add_chan();
void srv_hdr_ui_rm_chan();
void srv_hdr_ui_sys_msg();
void srv_hdr_ui_send_msg();
void srv_hdr_ui_recv_msg();
void srv_hdr_ui_recv_msg();

#endif /* __SRV_HDR_H */
