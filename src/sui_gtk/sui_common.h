#ifndef __UI_COMMON_H
#define __UI_COMMON_H

#include <gtk/gtk.h>

GtkListBoxRow* gtk_list_box_get_row_by_name(GtkListBox *listbox, const char* name);
GtkListBoxRow* gtk_list_box_add_unfocusable_row(GtkListBox *listbox, GtkWidget *widget);
GtkPopover* create_popover(GtkWidget *parent, GtkWidget *child, GtkPositionType pos);
char* show_open_filechosser(GtkWindow *parent);
void scale_size_to( int src_width, int src_height, int max_width, int max_height, int *dst_width, int *dst_height);

#endif /** __UI_COMMON_H **/
