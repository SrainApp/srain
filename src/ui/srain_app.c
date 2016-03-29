/**
 * @file srain_app.c
 * @brief Srain's application class implement
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include "ui.h"
#include "srain_app.h"
#include "meta.h"
#include "srain_window.h"
#include "rc.h"
#include "theme.h"

struct _SrainApp {
    GtkApplication parent;
};

struct _SrainAppClass {
    GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

static void srain_app_init(SrainApp *app){
    return;
}

static void srain_app_activate(GApplication *app){
    SrainWindow *win;

    theme_init();

    win = srain_window_new(SRAIN_APP(app));

    gtk_window_present(GTK_WINDOW(win));

    srain_window_add_chan(win, META_SERVER);

    ui_init(win);

    rc_read();
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate = srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP, "application-id", "org.gtk.srain", NULL);
}
