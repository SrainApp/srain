/**
 * @file srain_app.c
 * @brief Srain's application class implement
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "meta.h"
#include "rc.h"
#include "theme.h"
#include "server.h"
#include "server_cmd.h"

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

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

/* Only one SrainApp instance in one application */
SrainApp *app = NULL;

static void srain_app_init(SrainApp *self){
    if (app) return;

    self->server_join = (ServerJoinFunc)server_join;
    self->server_part = (ServerPartFunc)server_part;
    self->server_send = (ServerSendFunc)server_send;
    self->server_cmd = (ServerCmdFunc)server_cmd;

    app = self;

    return;
}

static void srain_app_activate(GtkApplication *app){
    GList *list;
    SrainWindow *win;

    list = gtk_application_get_windows(app);

    if (list){
        gtk_window_present(GTK_WINDOW(list->data));
    } else {
        theme_init();

        win = srain_window_new(SRAIN_APP(app));
        gtk_window_present(GTK_WINDOW(win));

        rc_read();
    }
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate = (void *)(GApplication *)srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP, "application-id", "org.gtk.srain", NULL);
}

void srain_app_join(const char *chan_name){
}

void srain_app_part(SrainBuffer *chan){
}

void srain_app_send(SrainBuffer *target, const char *msg){
}

int srain_app_cmd(SrainBuffer *source, const char *cmd){
    return 1;
}
