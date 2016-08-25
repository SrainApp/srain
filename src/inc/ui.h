#ifndef __UI_H
#define __UI_H

#include "srain_chat.h"
#include "srain_msg.h"
#include "srain_user_list.h"

/* Interface function pointers */
typedef void (*UIAddChatFunc)        (const char *server_name, const char *chat_name, const char *nick, ChatType type);
typedef void (*UIRmChatFunc)         (const char *server_name, const char *chat_name);
typedef void (*UISysMsgFunc)         (const char *server_name, const char *chat_name, const char *msg, SysMsgType type);
typedef void (*UISendMsgFunc)        (const char *server_name, const char *chat_name, const char *msg);
typedef void (*UIRecvMsgFunc)        (const char *server_name, const char *chat_name, const char *nick, const char *id, const char *msg);
typedef void (*UIUserListAddFunc)    (const char *server_name, const char *chat_name, const char *nick, UserType type);
typedef void (*UIUserListRmFunc)     (const char *server_name, const char *chat_name, const char *nick);
typedef void (*UIUserListRenameFunc) (const char *server_name, const char *chat_name, const char *old_nick, const char *new_nick, UserType type);
typedef void (*UISetTopicFunc)       (const char *server_name, const char *chat_name, const char *topic);

/* Macros for defining interface function */
#define DECLARE_UIAddChatFunc(func) void func (const char *server_name, const char *chat_name, const char *nick, ChatType type);
#define DECLARE_UIRmChatFunc(func) void func (const char *server_name, const char *chat_name);
#define DECLARE_UISysMsgFunc(func) void func (const char *server_name, const char *chat_name, const char *msg, SysMsgType type);
#define DECLARE_UISendMsgFunc(func) void func (const char *server_name, const char *chat_name, const char *msg);
#define DECLARE_UIRecvMsgFunc(func) void func (const char *server_name, const char *chat_name, const char *nick, const char *id, const char *msg);
#define DECLARE_UIUserListAddFunc(func) void func (const char *server_name, const char *chat_name, const char *nick, UserType type);
#define DECLARE_UIUserListRmFunc(func) void func (const char *server_name, const char *chat_name, const char *nick);
#define DECLARE_UIUserListRenameFunc(func) void func (const char *server_name, const char *chat_name, const char *old_nick, const char *new_nick, UserType type);
#define DECLARE_UISetTopicFunc(func) void func (const char *server_name, const char *chat_name, const char *topic);

void ui_init(int argc, char **argv);

/* UI interface functions, used by other module */
DECLARE_UIAddChatFunc(ui_add_chat);
DECLARE_UIRmChatFunc(ui_rm_chat);
DECLARE_UISysMsgFunc(ui_sys_msg);
DECLARE_UISendMsgFunc(ui_send_msg);
DECLARE_UIRecvMsgFunc(ui_recv_msg);
DECLARE_UIUserListAddFunc(ui_user_list_add);
DECLARE_UIUserListRmFunc(ui_user_list_rm);
DECLARE_UIUserListRenameFunc(ui_user_list_rename);
DECLARE_UISetTopicFunc(ui_set_topic);

/* Synchronous functions, export them for testing */
int ui_add_chat_sync(const char *srv_name, const char *chat_name, const char *nick, ChatType type);
int ui_rm_chat_sync(const char *srv_name, const char *chat_name);

void ui_sys_msg_sync(const char *srv_name, const char *chat_name, const char *msg, SysMsgType type);
void ui_send_msg_sync(const char *srv_name, const char *chat_name, const char *msg);
void ui_recv_msg_sync(const char *srv_name, const char *chat_name, const char *nick, const char *id, const char *msg);

int ui_user_list_add_sync(const char *srv_name, const char *chat_name, const char *nick, UserType type);
int ui_user_list_rm_sync(const char *srv_name, const char *chat_name, const char *nick);
void ui_user_list_rename_sync(const char *srv_name, const char *chat_name, const char *old_nick, const char *new_nick, UserType type);

void ui_set_topic_sync(const char *srv_name, const char *chat_name, const char *topic);

#endif /* __UI_H */
