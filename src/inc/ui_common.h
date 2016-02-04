#ifndef __UI_COMMON_H
#define __UI_COMMON_H

#include <gtk/gtk.h>

#define UI_BUILDER_GET_WIDGET(builder, widget) \
    widget = GTK_WIDGET(gtk_builder_get_object(builder, ""#widget"")); \
    assert(widget)

void detail_dialog_init(const char *name, const char *content);
void image_window_init(const gchar *path);

GtkWidget* get_widget_by_name(GtkWidget* widget, const gchar* name);
GtkListBoxRow* get_list_item_by_name(GtkListBox *listbox, const gchar* name);

#endif /** __UI_COMMON_H **/
