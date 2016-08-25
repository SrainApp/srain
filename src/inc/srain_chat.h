#ifndef __SRAIN_CHAN_H
#define __SRAIN_CHAN_H

#include <gtk/gtk.h>

#include "srain_msg.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_entry_completion.h"

#define SRAIN_TYPE_CHAN (srain_chat_get_type())
#define SRAIN_CHAN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAN, SrainChat))
#define SRAIN_IS_CHAN(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAN))

typedef enum {
    CHAT_SERVER,
    CHAT_CHANNEL,
    CHAT_PRIVATE,
    /* ... */
} ChatType;

typedef struct _SrainChat SrainChat;
typedef struct _SrainChatClass SrainChatClass;

GType srain_chat_get_type(void);
SrainChat* srain_chat_new(const char *srv_name, const char *chat_name, ChatType type);

void srain_chat_fcous_entry(SrainChat *chat);
void srain_chat_set_topic(SrainChat *chat, const char *topic);
void srain_chat_insert_text(SrainChat *chat, const char *text, int pos);
SrainUserList* srain_chat_get_user_list(SrainChat *chat);
SrainMsgList* srain_chat_get_msg_list(SrainChat *chat);
SrainEntryCompletion* srain_chat_get_entry_completion(SrainChat *chat);
const char* srain_chat_get_name(SrainChat *chat);
const char* srain_chat_get_srv_name(SrainChat *chat);
const char* srain_chat_get_chat_name(SrainChat *chat);
void srain_chat_set_nick(SrainChat *chat, const char *nick);
const char* srain_chat_get_nick(SrainChat *chat);
ChatType srain_chat_get_chat_type(SrainChat *chat);

#endif /* __SRAIN_CHAN_H */

