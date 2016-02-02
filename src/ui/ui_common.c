#include <string.h>
#include <gtk/gtk.h>

/* get a internal child widget by `name` in gtkContainer `widget`
 * you'd better to name the widget you want to find in glade file.
 * NOTE: only used for `msg_bubble_box`, so this function **ignore**
 * child inside `chat_msg_listbox` and `chat_online_box`
 */
GtkWidget* get_widget_by_name(GtkWidget* widget, const gchar* name){
    const gchar *widget_name;

    widget_name = gtk_widget_get_name(GTK_WIDGET(widget));
    if (strcmp(widget_name, (gchar*)name) == 0){
        return widget;
    }

    /* if this widget is the one which contain many childern we don't need, ignroe it */
    if (strcmp(widget_name, "chat_msg_listbox") == 0
            || strcmp(widget_name, "chat_online_box") == 0){
        return NULL;
    }

    if (GTK_IS_CONTAINER(widget)){
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
        while (children){
            GtkWidget* widget = get_widget_by_name(children->data, name);
            if (widget){
                return widget;
            }
            children = g_list_next(children);
        }
    }
    return NULL;
}

/* get a non-internal child widget by `name` in GtkListBox `widget`
 * return a GtkListBoxRow
 */
GtkListBoxRow* get_list_item_by_name(GtkListBox *listbox, const gchar* name){
    const gchar *widget_name;
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
