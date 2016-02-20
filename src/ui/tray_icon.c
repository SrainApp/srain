#include <gtk/gtk.h>
#include "log.h"

static void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data){
    LOG_FR("activate");
}

static void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button,
        guint activate_time, gpointer user_data){
    LOG_FR("popup-menu");
}

void tray_icon_set_callback(GtkStatusIcon *status_icon){
    g_signal_connect(G_OBJECT(status_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);
    g_signal_connect(G_OBJECT(status_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
}

