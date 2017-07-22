#ifndef __SRAIN_CHAT_H
#define __SRAIN_CHAT_H

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "srain_msg.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_entry_completion.h"

#define SRAIN_TYPE_CHAT (srain_chat_get_type())
#define SRAIN_CHAT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAT, SrainChat))
#define SRAIN_IS_CHAT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAT))

typedef enum {
    CHAT_SERVER,
    CHAT_CHANNEL,
    CHAT_PRIVATE,
    /* ... */
} ChatType;

typedef struct _SrainChat SrainChat;
typedef struct _SrainChatClass SrainChatClass;

GType srain_chat_get_type(void);
SrainChat* srain_chat_new(SuiSession *sui, const char *name, const char *remark, ChatType type);

void srain_chat_fcous_entry(SrainChat *chat);
void srain_chat_set_topic(SrainChat *chat, const char *topic);
void srain_chat_set_topic_setter(SrainChat *chat, const char *setter);
void srain_chat_insert_text(SrainChat *chat, const char *text, int pos);

SrainUserList* srain_chat_get_user_list(SrainChat *chat);
SrainMsgList* srain_chat_get_msg_list(SrainChat *chat);
SrainEntryCompletion* srain_chat_get_entry_completion(SrainChat *chat);
void srain_chat_set_name(SrainChat *chat, const char *name);
const char* srain_chat_get_name(SrainChat *chat);
void srain_chat_set_remark(SrainChat *chat, const char *remark);
const char* srain_chat_get_remark(SrainChat *chat);
void srain_chat_set_nick(SrainChat *chat, const char *nick);
const char* srain_chat_get_nick(SrainChat *chat);
ChatType srain_chat_get_chat_type(SrainChat *chat);
GtkMenu* srain_chat_get_menu(SrainChat *chat);
SuiSession *srain_chat_get_session(SrainChat *chat);

void srain_chat_show_topic(SrainChat *chat, bool isshow);
void srain_chat_show_user_list(SrainChat *chat, bool isshow);

#endif /* __SRAIN_CHAT_H */
