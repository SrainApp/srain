#ifndef __INTERFACE_H
#define __INTERFACE_H

/* For ChatType, SysMsgType, UserType, SrainMsgFlag */
#include "srain_chat.h"
#include "srain_msg.h"
#include "srain_user_list.h"

#define SIGN_UI_CHAT_ID const char *srv_name, const char *chat_name

#define SIGN_UI_ADD_CHAT  SIGN_UI_CHAT_ID, const char *nick, ChatType type
#define SIGN_UI_RM_CHAT   SIGN_UI_CHAT_ID
#define SIGN_UI_SYS_MSG   SIGN_UI_CHAT_ID, const char *msg, SysMsgType type, SrainMsgFlag flag
#define SIGN_UI_SEND_MSG  SIGN_UI_CHAT_ID, const char *msg, SrainMsgFlag flag
#define SIGN_UI_RECV_MSG  SIGN_UI_CHAT_ID, const char *nick, const char *id, const char *msg, SrainMsgFlag flag
#define SIGN_UI_ADD_USER  SIGN_UI_CHAT_ID, const char *nick, UserType type
#define SIGN_UI_RM_USER   SIGN_UI_CHAT_ID, const char *nick
#define SIGN_UI_REN_USER  SIGN_UI_CHAT_ID, const char *old_nick, const char *new_nick, UserType type
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

#define SIGN_SRV_CONNECT    const char *host, int port, const char *passwd, \
                            const char *nickname, const char *username, \
                            const char *realname, int ssl
#define SIGN_SRV_QUERY      const char *srv_name, const char *nick
#define SIGN_SRV_UNQUERY    const char *srv_name, const char *nick
#define SIGN_SRV_JOIN       const char *srv_name, const char *chan, const char *passwd
#define SIGN_SRV_PART       const char *srv_name, const char *chan
#define SIGN_SRV_QUIT       const char *srv_name, const char *reason
#define SIGN_SRV_SEND       const char *srv_name, const char *target, const char *msg
#define SIGN_SRV_CMD        const char *srv_name, const char *source, char *cmd, int block
#define SIGN_SRV_KICK       const char *srv_name, const char *nick, const char *chan, const char *reason
#define SIGN_SRV_WHOIS      const char *srv_name, const char *nick
#define SIGN_SRV_INVITE     const char *srv_name, const char *nick, const char *chan

typedef int (*SRVConnectFunc)   (SIGN_SRV_CONNECT);
typedef int (*SRVQuitFunc)      (SIGN_SRV_QUIT);
typedef int (*SRVQueryFunc)     (SIGN_SRV_QUERY);
typedef int (*SRVUnqueryFunc)   (SIGN_SRV_UNQUERY);
typedef int (*SRVJoinFunc)      (SIGN_SRV_JOIN);
typedef int (*SRVPartFunc)      (SIGN_SRV_PART);
typedef int (*SRVSendFunc)      (SIGN_SRV_SEND);
typedef int (*SRVCmdFunc)       (SIGN_SRV_CMD);
typedef int (*SRVKickFunc)      (SIGN_SRV_KICK);
typedef int (*SRVWhoisFunc)     (SIGN_SRV_WHOIS);
typedef int (*SRVInviteFunc)    (SIGN_SRV_INVITE);

#endif /* __INTERFACE_H */
