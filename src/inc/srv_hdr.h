#ifndef __SRV_HDR_H
#define __SRV_HDR_H

#include "ui.h"
#include "srv_session.h"

typedef void (*UIAddChanFunc) (const char *server_name, const char *chan_name);
typedef void (*UIRmChanFunc) (const char *server_name, const char *chan_name);
typedef void (*UISysMsgFunc) (const char *server_name, const char *chan_name, const char *msg, SysMsgType type);
typedef void (*UISendMsgFunc) (const char *server_name, const char *chan_name, const char *msg);
typedef void (*UIRecvMsgFunc) (const char *server_name, const char *chan_name, const char *nick, const char *id, const char *msg);
typedef void (*UIUserListAddFunc) (const char *server_name, const char *chan_name, const char *nick, UserType type, int notify);
typedef void (*UIUserListRmFunc) (const char *server_name, const char *chan_name, const char *nick, const char *reason);
typedef void (*UIUserListRenameFunc) (const char *server_name, const char *chan_name, const char *old_nick, const char *new_nick, UserType type);
typedef void (*UISetTopicFunc) (const char *server_name, const char *chan_name, const char *topic);

void srv_hdr_init();

/* UI handler for SRV module */
UIAddChanFunc srv_hdr_ui_add_chan;
UIRmChanFunc srv_hdr_ui_rm_chan;
UISysMsgFunc srv_hdr_ui_sys_msg;
UISendMsgFunc srv_hdr_ui_send_msg;
UIRecvMsgFunc srv_hdr_ui_recv_msg;
UIUserListAddFunc srv_hdr_ui_user_list_add;
UIUserListRmFunc srv_hdr_ui_user_list_rm;
UIUserListRenameFunc srv_hdr_ui_user_list_rename;
UISetTopicFunc srv_hdr_ui_set_topic;

#endif /* __SRV_HDR_H */
