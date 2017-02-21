/**
 * @file srain_app.c
 * @brief Srain's application class implement
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>

#include "theme.h"
#include "sui_common.h"
#include "srain_app.h"
#include "srain_window.h"

#include "srv.h"

#include "meta.h"
#include "rc.h"
#include "log.h"
#include "i18n.h"

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

/* Only one SrainApp instance in one application */
SrainApp *srain_app = NULL;
/* Only one SrainWindow instance in one application */
SrainWindow *srain_win = NULL;

static void srain_app_activate(GtkApplication *app){

    if (srain_win){
        gtk_window_present(GTK_WINDOW(srain_win));
    } else {
        srain_win = srain_window_new(SRAIN_APP(app));
        gtk_window_present(GTK_WINDOW(srain_win));

        Server *srv = server_new("ngircd1", "127.0.0.1", 6667, "", FALSE, "UTF-8", "LA", NULL, NULL);
        server_connect(srv);
        Server *srv2 = server_new("ngircd2", "127.0.0.1", 6667, "", FALSE, "UTF-8", "CC", NULL, NULL);
        if (srv2) server_connect(srv2);
    }
}

static void srain_app_init(SrainApp *self){
    if (srain_app) return;

    srain_app = self;
    return;
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate =
        (void *)(GApplication *)srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP,
            "application-id", "org.gtk.srain", NULL);
}

void srain_app_quit(SrainApp *app){
    GtkWidget *win;
    GList *list, *next;

    list = gtk_application_get_windows(GTK_APPLICATION(app));
    while (list){
        win = list->data;
        next = list->next;

        gtk_widget_destroy (GTK_WIDGET (win));

        list = next;
    }
}
