#ifndef __SRAIN_CHAN_H
#define __SRAIN_CHAN_H

#include <gtk/gtk.h>
#include "srain_msg.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_entry_completion.h"
#include "irc_magic.h"

#define SRAIN_TYPE_CHAN (srain_chan_get_type())
#define SRAIN_CHAN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAN, SrainChan))
#define SRAIN_IS_CHAN(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAN))

typedef struct _SrainChan SrainChan;
typedef struct _SrainChanClass SrainChanClass;

GType srain_chan_get_type(void);
SrainChan* srain_chan_new(const char *srv_name, const char *chan_name);

void srain_chan_fcous_entry(SrainChan *chan);
void srain_chan_set_topic(SrainChan *chan, const char *topic);
SrainUserList* srain_chan_get_user_list(SrainChan *chan);
SrainMsgList* srain_chan_get_msg_list(SrainChan *chan);
SrainEntryCompletion* srain_chan_get_entry_completion(SrainChan *chan);

#endif /* __SRAIN_CHAN_H */

