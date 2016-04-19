#ifndef __SRAIN_APP_H
#define __SRAIN_APP_H

#include <gtk/gtk.h>
#include "srain_chan.h"

#define SRAIN_TYPE_APP (srain_app_get_type())
#define SRAIN_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_APP, SrainApp))
typedef SrainChan SrainBuffer;

typedef struct _SrainApp SrainApp;
typedef struct _SrainAppClass SrainAppClass;

GType srain_app_get_type(void);
SrainApp *srain_app_new(void);

void srain_app_join(const char *chan_name);
void srain_app_part(SrainBuffer *chan);
void srain_app_send(SrainBuffer *target, const char *msg);
int srain_app_cmd(SrainBuffer *source, const char *cmd);

#endif /* __SRAIN_APP_H */
