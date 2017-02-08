#ifndef __UI_H
#define __UI_H

#include "srain_chat.h"
#include "srain_msg.h"
#include "srain_user_list.h"

typedef struct {
    char *name;
    char *remark;

    SrainChat *chat;
    void *ctx;
} SuiSession;

void ui_init(int argc, char **argv);
void* ui_add_chat(Server *srv, const char *chat_name, ChatType type);
void ui_rm_chat(void *chat);
void ui_sys_msg(void *chat, const char *msg, SysMsgType type, SrainMsgFlag flag);
void ui_send_msg(void *chat, const char *msg, SrainMsgFlag flag);
void ui_recv_msg(void *chat, const char *nick, const char *id, const char *msg, SrainMsgFlag flag);
int ui_add_user(void *chat, const char *nick, UserType type);
int ui_rm_user(void *chat, const char *nick);
void ui_ren_user(void *chat, const char *old_nick, const char *new_nick, UserType type);
void ui_set_topic(void *chat, const char *topic);

#endif /* __UI_H */
