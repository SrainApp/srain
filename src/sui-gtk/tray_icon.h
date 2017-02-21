#ifndef __TRAY_ICON_H
#define __TRAY_ICON_H

#include <gtk/gtk.h>
#include "srain_window.h"

void tray_icon_set_callback(GtkStatusIcon *status_icon, SrainWindow *win, GtkMenu *menu);

#endif /* __TRAY_ICON_H */
