#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

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

static void apply_css(GtkWidget *widget, GtkStyleProvider *provider){
    //  gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider, G_MAXUINT);

    if(GTK_IS_CONTAINER(widget))
        gtk_container_forall(GTK_CONTAINER(widget),(GtkCallback)apply_css, provider);
}
