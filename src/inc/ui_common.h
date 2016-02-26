#ifndef __UI_COMMON_H
#define __UI_COMMON_H

#include <gtk/gtk.h>

void get_cur_time(char *timestr);
GtkListBoxRow* get_list_item_by_name(GtkListBox *listbox, const gchar* name);

#endif /** __UI_COMMON_H **/
