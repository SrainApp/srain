#ifndef __UI_H
#define __UI_H

#include "interfaces.h"

#include "srain_chat.h"
#include "srain_msg.h"
#include "srain_user_list.h"

void ui_init(int argc, char **argv);

/* UI interface functions, used by other module */
void ui_add_chat    (SIGN_UI_ADD_CHAT);
void ui_rm_chat     (SIGN_UI_RM_CHAT);
void ui_sys_msg     (SIGN_UI_SYS_MSG);
void ui_send_msg    (SIGN_UI_SEND_MSG);
void ui_recv_msg    (SIGN_UI_RECV_MSG);
void ui_add_user    (SIGN_UI_ADD_USER);
void ui_rm_user     (SIGN_UI_RM_USER);
void ui_ren_user    (SIGN_UI_REN_USER);
void ui_set_topic   (SIGN_UI_SET_TOPIC);

/* Synchronous functions, export them for testing */
int ui_add_chat_sync    (SIGN_UI_ADD_CHAT);
int ui_rm_chat_sync     (SIGN_UI_RM_CHAT);
void ui_sys_msg_sync    (SIGN_UI_SYS_MSG);
void ui_send_msg_sync   (SIGN_UI_SEND_MSG);
void ui_recv_msg_sync   (SIGN_UI_RECV_MSG);
int ui_add_user_sync    (SIGN_UI_ADD_USER);
int ui_rm_user_sync     (SIGN_UI_RM_USER);
void ui_ren_user_sync   (SIGN_UI_REN_USER);
void ui_set_topic_sync  (SIGN_UI_SET_TOPIC);

#endif /* __UI_H */
