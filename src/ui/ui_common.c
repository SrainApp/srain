/**
 * @file ui_common.c
 * @brief useful functions required by UI module
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>
#include "theme.h"
#include "i18n.h"

/**
 * @brief get current time
 *
 * @param a pointer to store time string
 */
void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, 32, "%H:%M", localtime(&curtime));
    timestr[31] = '\0';
}

/**
 * @brief get a non-internal child widget by `name` in GtkListBox `widget`
 *
 * @param listbox a GtkListBox
 * @param name the name of child widget that you want to find
 *
 * @return a GtkListRow if found, or return NULL
 */
GtkListBoxRow* gtk_list_box_get_row_by_name(GtkListBox *listbox, const gchar* name){
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

static void filechooser_on_update_preview(GtkFileChooser *chooser,
        gpointer user_data){
    char *filename;
    GError *error;
    GtkImage *preview;
    GdkPixbuf *pixbuf;

    filename = gtk_file_chooser_get_preview_filename(chooser);
    preview = GTK_IMAGE(gtk_file_chooser_get_preview_widget(chooser));
    pixbuf = gdk_pixbuf_new_from_file_at_size(filename, 300, 300, &error);

    if (error == NULL){
        gtk_image_set_from_pixbuf(preview, pixbuf);
        g_object_unref(pixbuf);
    } else {
        gtk_image_clear(preview);
    }

    g_free(filename);
}


/**
 * @brief show_open_filechosser
 *
 * @param parent dialog parent
 *
 * @return NULL or a filename, filename must be freed
 * with g_free()
 *
 * GtkFileChooser wrapper for opening file with image preview
 */
char* show_open_filechosser(GtkWindow *parent){
    int res;
    char *filename;
    GtkImage *preview;
    GtkFileChooserDialog *dialog;
    GtkFileChooser *chooser;

    dialog = GTK_FILE_CHOOSER_DIALOG(
            gtk_file_chooser_dialog_new("Open File",
                parent, GTK_FILE_CHOOSER_ACTION_OPEN,
                _("cancel"), GTK_RESPONSE_CANCEL,
                _("open"), GTK_RESPONSE_ACCEPT,
                NULL));
    chooser = GTK_FILE_CHOOSER(dialog);
    preview = GTK_IMAGE(gtk_image_new());

    gtk_widget_show(GTK_WIDGET(preview));
    gtk_file_chooser_set_preview_widget(chooser, GTK_WIDGET(preview));

    g_signal_connect(chooser, "update-preview",
            G_CALLBACK(filechooser_on_update_preview), NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    filename = NULL;
    if(res == GTK_RESPONSE_ACCEPT){
        filename = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
    // gtk_widget_destroy(GTK_WIDGET(preview));

    return filename;
}

/**
 * @brief gtk_list_box_add_item
 *
 * @param listbox
 * @param widget
 *
 * @return a unfocusable GtkListRow
 *
 * a useful function used to add unfocusable row which contain `widget`
 * into `listbox`
 */

GtkListBoxRow* gtk_list_box_add_unfocusable_row(GtkListBox *listbox, GtkWidget *widget){
    GtkListBoxRow *row;

    row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());

    gtk_widget_set_can_focus(widget, FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(row), FALSE);

    gtk_container_add(GTK_CONTAINER(row), widget);
    gtk_list_box_insert(listbox, GTK_WIDGET(row), -1);

    gtk_widget_show(GTK_WIDGET(row));
    gtk_widget_show(widget);

    theme_apply(GTK_WIDGET(row));

    return row;
}
