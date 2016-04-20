#ifndef __SRAIN_APP_H
#define __SRAIN_APP_H

#include <gtk/gtk.h>
#include "srain_chan.h"

#define SRAIN_TYPE_APP (srain_app_get_type())
#define SRAIN_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_APP, SrainApp))

typedef int (*ServerJoinFunc) (void *server, const char *chan_name);
typedef int (*ServerPartFunc) (void *server, const char *chan_name);
typedef int (*ServerSendFunc) (void *server, const char *target, const char *msg);
typedef int (*ServerCmdFunc) (void *server, const char *source, const char *cmd);

struct _SrainApp {
    GtkApplication parent;

    ServerJoinFunc server_join;
    ServerPartFunc server_part;
    ServerSendFunc server_send;
    ServerCmdFunc server_cmd;
};

struct _SrainAppClass {
    GtkApplicationClass parent_class;
};

typedef struct _SrainApp SrainApp;
typedef struct _SrainAppClass SrainAppClass;

GType srain_app_get_type(void);
SrainApp *srain_app_new(void);

void srain_app_join(const char *chan_name);
void srain_app_part(SrainChan *chan);
void srain_app_send(SrainChan *target, const char *msg);
int srain_app_cmd(SrainChan *source, const char *cmd);

#endif /* __SRAIN_APP_H */
