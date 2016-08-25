#ifndef __SRAIN_CHAN_H
#define __SRAIN_CHAN_H

#include <gtk/gtk.h>

#include "srain_msg.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_entry_completion.h"

#define SRAIN_TYPE_CHAN (srain_chan_get_type())
#define SRAIN_CHAN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAN, SrainChan))
#define SRAIN_IS_CHAN(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAN))

typedef enum {
    CHAT_SERVER,
    CHAT_CHANNEL,
    CHAT_PRIVATE,
    /* ... */
} ChatType;

typedef struct _SrainChan SrainChan;
typedef struct _SrainChanClass SrainChanClass;

GType srain_chan_get_type(void);
SrainChan* srain_chan_new(const char *srv_name, const char *chan_name, ChatType type);

void srain_chan_fcous_entry(SrainChan *chan);
void srain_chan_set_topic(SrainChan *chan, const char *topic);
void srain_chan_insert_text(SrainChan *chan, const char *text, int pos);
SrainUserList* srain_chan_get_user_list(SrainChan *chan);
SrainMsgList* srain_chan_get_msg_list(SrainChan *chan);
SrainEntryCompletion* srain_chan_get_entry_completion(SrainChan *chan);
const char* srain_chan_get_name(SrainChan *chan);
const char* srain_chan_get_srv_name(SrainChan *chan);
const char* srain_chan_get_chan_name(SrainChan *chan);
void srain_chan_set_nick(SrainChan *chan, const char *nick);
const char* srain_chan_get_nick(SrainChan *chan);
ChatType srain_chan_get_chat_type(SrainChan *chan);

#endif /* __SRAIN_CHAN_H */

