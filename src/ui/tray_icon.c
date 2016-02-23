#include <gtk/gtk.h>
#include "log.h"

static void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data){
    GtkWidget *win = user_data;
    static gboolean is_visible = TRUE;

    if (is_visible)
        gtk_widget_hide(win);
    else 
        gtk_widget_show(win);

    LOG_FR("%s", is_visible?"hide":"show");
    is_visible = !is_visible;
}

static void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button,
        guint activate_time, gpointer user_data){
    LOG_FR("popup-menu");
}

void tray_icon_set_callback(GtkStatusIcon *status_icon, GtkWidget *win){
    g_signal_connect(G_OBJECT(status_icon), "activate", G_CALLBACK(tray_icon_on_click), win);
    g_signal_connect(G_OBJECT(status_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
}

