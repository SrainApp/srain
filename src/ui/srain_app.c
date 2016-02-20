#include <gtk/gtk.h>

#include "srain_app.h"
#include "srain_window.h"

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

    win = srain_window_new(SRAIN_APP(app));
    gtk_window_set_title(GTK_WINDOW(win), "Srain");
    gtk_window_present(GTK_WINDOW(win));
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate = srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP, "application-id", "org.gtk.srain", NULL);
}
