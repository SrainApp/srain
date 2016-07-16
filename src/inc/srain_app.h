#ifndef __SRAIN_APP_H
#define __SRAIN_APP_H

#include <gtk/gtk.h>
#include "srain_chan.h"

#define SRAIN_TYPE_APP (srain_app_get_type())
#define SRAIN_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_APP, SrainApp))

typedef int (*ServerJoinFunc) (const char *server, const char *chan_name);
typedef int (*ServerPartFunc) (const char *server, const char *chan_name);
typedef int (*ServerSendFunc) (const char *server, const char *target, const char *msg);
typedef int (*ServerCmdFunc) (const char *server, const char *source, const char *cmd);

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
void srain_app_quit(SrainApp *app);

/* Only one SrainApp instance in one application */
extern SrainApp *srain_app;

#endif /* __SRAIN_APP_H */
