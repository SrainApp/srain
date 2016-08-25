#ifndef __SRV_HDR_H
#define __SRV_HDR_H

#include "ui.h"
#include "srv_session.h"

void srv_hdr_init();

/* UI handler for SRV module */
UIAddChatFunc srv_hdr_ui_add_chat;
UIRmChatFunc srv_hdr_ui_rm_chat;
UISysMsgFunc srv_hdr_ui_sys_msg;
UISendMsgFunc srv_hdr_ui_send_msg;
UIRecvMsgFunc srv_hdr_ui_recv_msg;
UIUserListAddFunc srv_hdr_ui_user_list_add;
UIUserListRmFunc srv_hdr_ui_user_list_rm;
UIUserListRenameFunc srv_hdr_ui_user_list_rename;
UISetTopicFunc srv_hdr_ui_set_topic;

#endif /* __SRV_HDR_H */
