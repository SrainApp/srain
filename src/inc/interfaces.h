#ifndef __INTERFACE_H
#define __INTERFACE_H

#define SIGN_UI_CHAT_ID const char *srv_name, const char *chat_name

#define SIGN_UI_ADD_CHAT  SIGN_UI_CHAT_ID, const char *nick, int type
#define SIGN_UI_RM_CHAT   SIGN_UI_CHAT_ID
#define SIGN_UI_SYS_MSG   SIGN_UI_CHAT_ID, const char *msg, int type, int flag
#define SIGN_UI_SEND_MSG  SIGN_UI_CHAT_ID, const char *msg, int flag
#define SIGN_UI_RECV_MSG  SIGN_UI_CHAT_ID, const char *nick, const char *id, const char *msg, int flag
#define SIGN_UI_ADD_USER  SIGN_UI_CHAT_ID, const char *nick, int type
#define SIGN_UI_RM_USER   SIGN_UI_CHAT_ID, const char *nick
#define SIGN_UI_REN_USER  SIGN_UI_CHAT_ID, const char *old_nick, const char *new_nick, int type
#define SIGN_UI_SET_TOPIC SIGN_UI_CHAT_ID, const char *topic

typedef void (*UIAddChatFunc)   (SIGN_UI_ADD_CHAT);
typedef void (*UIRmChatFunc)    (SIGN_UI_RM_CHAT);
typedef void (*UISysMsgFunc)    (SIGN_UI_SYS_MSG);
typedef void (*UISendMsgFunc)   (SIGN_UI_SEND_MSG);
typedef void (*UIRecvMsgFunc)   (SIGN_UI_RECV_MSG);
typedef void (*UIAddUserFunc)   (SIGN_UI_ADD_USER);
typedef void (*UIRmUserFunc)    (SIGN_UI_RM_USER);
typedef void (*UIRenUserFunc)   (SIGN_UI_REN_USER);
typedef void (*UISetTopicFunc)  (SIGN_UI_SET_TOPIC);

#define SIGN_SRV_JOIN const char *srv_name, const char *chan_name, const char *passwd
#define SIGN_SRV_PART const char *srv_name, const char *chan_name
#define SIGN_SRV_SEND const char *srv_name, const char *target, const char *msg
#define SIGN_SRV_CMD const char *srv_name, const char *source, char *cmd, int block

typedef int (*SRVJoinFunc) (SIGN_SRV_JOIN);
typedef int (*SRVPartFunc) (SIGN_SRV_PART);
typedef int (*SRVSendFunc) (SIGN_SRV_SEND);
typedef int (*SRVCmdFunc)  (SIGN_SRV_CMD);

#endif /* __INTERFACE_H */
