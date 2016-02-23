#ifndef __SRAIN_CHANBOX_H
#define __SRAIN_CHANBOX_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_CHAN (srain_chan_get_type())
#define SRAIN_CHAN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAN, SrainChan))
#define SRAIN_IS_CHAN(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAN))

typedef struct _SrainChan SrainChan;
typedef struct _SrainChanClass SrainChanClass;

GType srain_chan_get_type(void);
SrainChan *srain_chan_new(const char *name);
void srain_chan_set_name(SrainChan *chan, const char *name);
void srain_chan_set_topic(SrainChan *chan, const char *topic);
void srain_chan_online_list_rm(SrainChan *chan, const char *name);
void srain_chan_online_list_add(SrainChan *chan, const char *name);

#endif /* __SRAIN_CHAN_H */

