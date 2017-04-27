/**
 * @file ui_common.c
 * @brief useful functions required by UI module
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "srain_window.h"

#include "theme.h"
#include "i18n.h"

extern SrainWindow *srain_win;

/**
 * @brief get a non-internal child widget by `name` in GtkListBox `widget`
 *
 * @param listbox a GtkListBox
 * @param name the name of child widget that you want to find
 *
 * @return a GtkListRow if found, or return NULL
 */
GtkListBoxRow* gtk_list_box_get_row_by_name(GtkListBox *listbox, const char* name){
    const char *widget_name;
    GList *rows = gtk_container_get_children(GTK_CONTAINER(listbox));

    while (rows){
        widget_name = gtk_widget_get_name(rows->data);
        if (strcmp(widget_name, name) == 0){
            return rows->data;
        }
        rows = g_list_next(rows);
    }
    return NULL;
}

static void filechooser_on_update_preview(GtkFileChooser *chooser,
        gpointer user_data){
    char *filename;
    GError *error = NULL;
    GtkImage *preview;
    GdkPixbuf *pixbuf;

    filename = gtk_file_chooser_get_preview_filename(chooser);
    preview = GTK_IMAGE(gtk_file_chooser_get_preview_widget(chooser));
    if (!filename || !preview) return;

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
            gtk_file_chooser_dialog_new(_("Open File"),
                parent, GTK_FILE_CHOOSER_ACTION_OPEN,
                _("Cancel"), GTK_RESPONSE_CANCEL,
                _("Open"), GTK_RESPONSE_ACCEPT,
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

void show_msg_dialog(const char *title, const char *msg){
    GtkMessageDialog *dia = GTK_MESSAGE_DIALOG(
            gtk_message_dialog_new(GTK_WINDOW(srain_win),
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                NULL
                )
            );

    gtk_window_set_title(GTK_WINDOW(dia), title);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dia), msg);

    /* Without this, message dialog cannot be displayed on the center */
    while (gtk_events_pending()) gtk_main_iteration();

    gtk_dialog_run(GTK_DIALOG(dia));
    gtk_widget_destroy(GTK_WIDGET(dia));
}

void scale_size_to(int src_width, int src_height,
        int max_width, int max_height, int *dst_width, int *dst_height){
    long double src_ratio;
    long double max_ratio;

    if (!src_width || !src_height) return;
    if (!max_width || !max_height) return;
    if (!dst_width || !dst_height) return;

    src_ratio = src_width * 1.0 / src_height;
    max_ratio = max_width * 1.0 / max_height;

    if (src_width > max_width || src_height > max_height){
        if (src_ratio > max_ratio){
            /* 相同宽度下 src 有更大的高度，需要调整 */
            *dst_width = max_width;
            *dst_height = (int)(max_height / src_ratio);
        } else {
            /* 相同高度下 src 有更大的宽度，需要调整 */
            *dst_width = (int)(max_height * src_ratio);
            *dst_height = max_height;
        }
    } else {
        *dst_width = src_width;
        *dst_height = src_height;
    }
}
