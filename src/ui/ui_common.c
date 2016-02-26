#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

/* */
void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, 32, "%m-%d %H:%M", localtime(&curtime));
    timestr[31] = '\0';
}

/* get a non-internal child widget by `name` in GtkListBox `widget`
 * return a GtkListBoxRow
 */
GtkListBoxRow* get_list_item_by_name(GtkListBox *listbox, const char* name){
    const char *widget_name;
    GtkWidget *item;
    GList *row = gtk_container_get_children(GTK_CONTAINER(listbox));

    while (row){
        item = gtk_bin_get_child(GTK_BIN(row->data));
        widget_name = gtk_widget_get_name(item);
        if (strcmp(widget_name, name) == 0){
            return row->data;
        }
        row = g_list_next(row);
    }
    return NULL;
}
