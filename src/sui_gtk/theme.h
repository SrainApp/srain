#ifndef __THEME_H
#define __THEME_H

#include <gtk/gtk.h>

int theme_load(const char *theme);
void theme_apply(GtkWidget *widget);

#endif /* __THEME_H */
