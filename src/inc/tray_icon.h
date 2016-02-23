#ifndef __TRAY_ICON_H
#define __TRAY_ICON_H

#include <gtk/gtk.h>

void tray_icon_set_callback(GtkStatusIcon *status_icon, GtkWidget *win);

#endif /* __TRAY_ICON_H */
