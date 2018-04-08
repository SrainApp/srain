/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file tray_icon.c
 * @brief System tray's callback functions
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 *
 * system tray is defined in data/ui/window.glade,
 * and binded in src/ui/srain_window.c::srain_window_class_init()
 * so only callback functions are provided here.
 *
 */

#include <gtk/gtk.h>
#include "sui_window.h"
#include "log.h"

GtkMenu *tray_menu;

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
    gtk_menu_popup(tray_menu, NULL, NULL, NULL, NULL, button, activate_time);
}

void tray_icon_set_callback(GtkStatusIcon *status_icon, SuiWindow *win, GtkMenu *menu){
    tray_menu = menu;

    g_signal_connect(G_OBJECT(status_icon), "activate", G_CALLBACK(tray_icon_on_click), win);
    g_signal_connect(G_OBJECT(status_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);
}
