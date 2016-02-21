#ifndef __SRAIN_CHANBOX_H
#define __SRAIN_CHANBOX_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_CHAN_BOX (srain_chan_box_get_type())
#define SRAIN_CHAN_BOX(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_CHAN_BOX, SrainChanBox))
#define SRAIN_IS_CHAN_BOX(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_CHAN_BOX))

typedef struct _SrainChanBox SrainChanBox;
typedef struct _SrainChanBoxClass SrainChanBoxClass;

GType srain_chan_box_get_type(void);
SrainChanBox *srain_chan_box_new(const char *name);

#endif /* __CHAN_BOX_H */

