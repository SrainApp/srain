#include <gtk/gtk.h>
#include "ui.h"
#include "theme.h"
#include "srain_app.h"
#include "srain_window.h"
#include "config.h"

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

    theme_apply(GTK_WIDGET(win));
    gtk_window_present(GTK_WINDOW(win));

    SrainChan *chan = srain_window_add_chan(win, "*server*");
    srain_chan_set_topic(chan, "==== TOPIC ====");
    ui_init(win);
    config_read();
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate = srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP, "application-id", "org.gtk.srain", NULL);
}
