/**
 * @file srain_app.c
 * @brief Srain's application class implement
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>

#include "theme.h"
#include "ui_test.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_user_list.h"
#include "srain_msg_list.h"
#include "srain_entry_completion.h"

#include "server.h"
#include "server_cmd.h"

#include "meta.h"
#include "rc.h"
#include "log.h"
#include "i18n.h"

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

/* Only one SrainApp instance in one application */
SrainApp *srain_app = NULL;
/* Only one SrainWindow instance in one application */
SrainWindow *srain_win = NULL;

static void srain_app_init(SrainApp *self){
    if (srain_app) return;

#ifdef UI_TEST
    self->server_join = (ServerJoinFunc)ui_test_server_join;
    self->server_part = (ServerPartFunc)ui_test_server_part;
    self->server_send = (ServerSendFunc)ui_test_server_send;
    self->server_cmd = (ServerCmdFunc)ui_test_server_cmd;
#else
    self->server_join = (ServerJoinFunc)server_join;
    self->server_part = (ServerPartFunc)server_part;
    self->server_send = (ServerSendFunc)server_send;
    self->server_cmd = (ServerCmdFunc)server_cmd;
#endif

    srain_app = self;

    return;
}

static void srain_app_activate(GtkApplication *app){
    if (srain_win){
        gtk_window_present(GTK_WINDOW(srain_win));
    } else {
        theme_init();
        srain_win = srain_window_new(SRAIN_APP(app));
        gtk_window_present(GTK_WINDOW(srain_win));
#ifdef UI_TEST
        ui_test();
#else
        rc_read();
#endif
    }
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate =
        (void *)(GApplication *)srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP,
            "application-id", "org.gtk.srain", NULL);
}
