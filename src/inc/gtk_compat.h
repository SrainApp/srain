/* Copyright (C) 2026
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GTK_COMPAT_H
#define __GTK_COMPAT_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef void (*SrnGtkWidgetForeachFunc)(GtkWidget *widget, gpointer user_data);

static inline void srn_gtk_widget_add_css_class(GtkWidget *widget,
        const char *css_class){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_add_css_class(widget, css_class);
#else
    GtkStyleContext *style_context;

    style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(style_context, css_class);
#endif
}

static inline void srn_gtk_widget_remove_css_class(GtkWidget *widget,
        const char *css_class){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_remove_css_class(widget, css_class);
#else
    GtkStyleContext *style_context;

    style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_remove_class(style_context, css_class);
#endif
}

static inline void srn_gtk_widget_show(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_set_visible(widget, TRUE);
#else
    gtk_widget_show(widget);
#endif
}

static inline void srn_gtk_widget_hide(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_set_visible(widget, FALSE);
#else
    gtk_widget_hide(widget);
#endif
}

static inline void srn_gtk_image_set_icon_name(GtkImage *image,
        const char *icon_name){
#if GTK_MAJOR_VERSION >= 4
    gtk_image_set_from_icon_name(image, icon_name);
#else
    gtk_image_set_from_icon_name(image, icon_name, GTK_ICON_SIZE_BUTTON);
#endif
}

static inline void srn_gtk_image_set_from_pixbuf(GtkImage *image,
        GdkPixbuf *pixbuf){
#if GTK_MAJOR_VERSION >= 4
    GdkTexture *texture;

    if (!pixbuf){
        gtk_image_clear(image);
        return;
    }

    texture = gdk_texture_new_for_pixbuf(pixbuf);
    gtk_image_set_from_paintable(image, GDK_PAINTABLE(texture));
    g_object_unref(texture);
#else
    gtk_image_set_from_pixbuf(image, pixbuf);
#endif
}

static inline GtkWidget *srn_gtk_button_get_child(GtkButton *button){
#if GTK_MAJOR_VERSION >= 4
    return gtk_button_get_child(button);
#else
    return gtk_button_get_image(button);
#endif
}

static inline GtkPopover *srn_gtk_popover_new_from_menu_model(
        GMenuModel *model){
#if GTK_MAJOR_VERSION >= 4
    return GTK_POPOVER(gtk_popover_menu_new_from_model(model));
#else
    return GTK_POPOVER(gtk_popover_new_from_model(NULL, model));
#endif
}

static inline void srn_gtk_popover_set_relative_to(GtkPopover *popover,
        GtkWidget *relative_to){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_set_parent(GTK_WIDGET(popover), relative_to);
#else
    gtk_popover_set_relative_to(popover, relative_to);
#endif
}

static inline GtkWidget *srn_gtk_menu_item_new_with_mnemonic(
        const char *label){
#if GTK_MAJOR_VERSION >= 4
    return gtk_button_new_with_mnemonic(label);
#else
    return gtk_menu_item_new_with_mnemonic(label);
#endif
}

static inline GtkWidget *srn_gtk_menu_item_new_with_label(
        const char *label){
#if GTK_MAJOR_VERSION >= 4
    return gtk_button_new_with_label(label);
#else
    return gtk_menu_item_new_with_label(label);
#endif
}

static inline GtkWidget *srn_gtk_check_menu_item_new_with_mnemonic(
        const char *label){
#if GTK_MAJOR_VERSION >= 4
    return gtk_check_button_new_with_mnemonic(label);
#else
    return gtk_check_menu_item_new_with_mnemonic(label);
#endif
}

static inline GtkWidget *srn_gtk_menu_new(void){
#if GTK_MAJOR_VERSION >= 4
    return gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    return gtk_menu_new();
#endif
}

static inline GtkWidget *srn_gtk_event_box_new(void){
#if GTK_MAJOR_VERSION >= 4
    return gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
    return gtk_event_box_new();
#endif
}

static inline void srn_gtk_menu_append(GtkWidget *menu, GtkWidget *item){
#if GTK_MAJOR_VERSION >= 4
    gtk_box_append(GTK_BOX(menu), item);
#else
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
#endif
}

static inline void srn_gtk_menu_popup(GtkWidget *menu, gpointer event){
#if GTK_MAJOR_VERSION >= 4
    (void)menu;
    (void)event;
    g_assert_not_reached();
#else
    gtk_menu_popup_at_pointer(GTK_MENU(menu), event);
#endif
}

static inline gulong srn_gtk_menu_item_connect_activate(GtkWidget *item,
        GCallback callback, gpointer user_data){
#if GTK_MAJOR_VERSION >= 4
    return g_signal_connect(item, "clicked", callback, user_data);
#else
    return g_signal_connect(item, "activate", callback, user_data);
#endif
}

static inline void srn_gtk_check_menu_item_set_active(GtkWidget *item,
        bool active){
#if GTK_MAJOR_VERSION >= 4
    gtk_check_button_set_active(GTK_CHECK_BUTTON(item), active);
#else
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), active);
#endif
}

static inline bool srn_gtk_check_menu_item_get_active(GtkWidget *item){
#if GTK_MAJOR_VERSION >= 4
    return gtk_check_button_get_active(GTK_CHECK_BUTTON(item));
#else
    return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));
#endif
}

static inline void srn_gtk_check_button_set_active(GtkCheckButton *button,
        bool active){
#if GTK_MAJOR_VERSION >= 4
    gtk_check_button_set_active(button, active);
#else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), active);
#endif
}

static inline bool srn_gtk_check_button_get_active(GtkCheckButton *button){
#if GTK_MAJOR_VERSION >= 4
    return gtk_check_button_get_active(button);
#else
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
#endif
}

static inline void srn_gtk_check_menu_item_toggled(GtkWidget *item){
#if GTK_MAJOR_VERSION >= 4
    g_signal_emit_by_name(item, "toggled");
#else
    gtk_check_menu_item_toggled(GTK_CHECK_MENU_ITEM(item));
#endif
}

static inline void srn_gtk_widget_add_child(GtkWidget *parent,
        GtkWidget *child){
#if GTK_MAJOR_VERSION >= 4
    if (GTK_IS_BOX(parent)){
        gtk_box_append(GTK_BOX(parent), child);
    }
    else if (GTK_IS_SCROLLED_WINDOW(parent)){
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(parent), child);
    }
    else if (GTK_IS_REVEALER(parent)){
        gtk_revealer_set_child(GTK_REVEALER(parent), child);
    }
    else if (GTK_IS_LIST_BOX_ROW(parent)){
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(parent), child);
    }
    else if (GTK_IS_POPOVER(parent)){
        gtk_popover_set_child(GTK_POPOVER(parent), child);
    }
    else if (GTK_IS_HEADER_BAR(parent)){
        gtk_header_bar_pack_end(GTK_HEADER_BAR(parent), child);
    }
    else if (GTK_IS_WINDOW(parent)){
        gtk_window_set_child(GTK_WINDOW(parent), child);
    }
    else {
        g_assert_not_reached();
    }
#else
    gtk_container_add(GTK_CONTAINER(parent), child);
#endif
}

static inline void srn_gtk_widget_remove_child(GtkWidget *parent,
        GtkWidget *child){
#if GTK_MAJOR_VERSION >= 4
    if (GTK_IS_BOX(parent)){
        gtk_box_remove(GTK_BOX(parent), child);
    }
    else if (GTK_IS_LIST_BOX(parent)){
        gtk_list_box_remove(GTK_LIST_BOX(parent), child);
    }
    else if (GTK_IS_STACK(parent)){
        gtk_stack_remove(GTK_STACK(parent), child);
    }
    else if (GTK_IS_HEADER_BAR(parent)){
        gtk_header_bar_remove(GTK_HEADER_BAR(parent), child);
    }
    else if (GTK_IS_POPOVER(parent) &&
            gtk_popover_get_child(GTK_POPOVER(parent)) == child){
        gtk_popover_set_child(GTK_POPOVER(parent), NULL);
    }
    else if (GTK_IS_WINDOW(parent) &&
            gtk_window_get_child(GTK_WINDOW(parent)) == child){
        gtk_window_set_child(GTK_WINDOW(parent), NULL);
    }
    else if (GTK_IS_REVEALER(parent) &&
            gtk_revealer_get_child(GTK_REVEALER(parent)) == child){
        gtk_revealer_set_child(GTK_REVEALER(parent), NULL);
    }
    else {
        g_assert_not_reached();
    }
#else
    gtk_container_remove(GTK_CONTAINER(parent), child);
#endif
}

static inline GtkWidget *srn_gtk_widget_get_child(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    if (GTK_IS_LIST_BOX_ROW(widget)){
        return gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(widget));
    }
    if (GTK_IS_POPOVER(widget)){
        return gtk_popover_get_child(GTK_POPOVER(widget));
    }
    if (GTK_IS_SCROLLED_WINDOW(widget)){
        return gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(widget));
    }
    if (GTK_IS_REVEALER(widget)){
        return gtk_revealer_get_child(GTK_REVEALER(widget));
    }
    return gtk_widget_get_first_child(widget);
#else
    return gtk_bin_get_child(GTK_BIN(widget));
#endif
}

static inline GList *srn_gtk_widget_get_children(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    GList *children;
    GtkWidget *child;

    children = NULL;
    child = gtk_widget_get_first_child(widget);
    while (child){
        children = g_list_append(children, child);
        child = gtk_widget_get_next_sibling(child);
    }
    return children;
#else
    return gtk_container_get_children(GTK_CONTAINER(widget));
#endif
}

static inline void srn_gtk_widget_foreach_child(GtkWidget *widget,
        SrnGtkWidgetForeachFunc callback, gpointer user_data){
#if GTK_MAJOR_VERSION >= 4
    for (GtkWidget *child = gtk_widget_get_first_child(widget);
            child != NULL;
            child = gtk_widget_get_next_sibling(child)){
        callback(child, user_data);
    }
#else
    gtk_container_foreach(GTK_CONTAINER(widget), callback, user_data);
#endif
}

static inline const char *srn_gtk_entry_get_text(GtkEntry *entry){
#if GTK_MAJOR_VERSION >= 4
    return gtk_editable_get_text(GTK_EDITABLE(entry));
#else
    return gtk_entry_get_text(entry);
#endif
}

static inline void srn_gtk_entry_set_text(GtkEntry *entry, const char *text){
#if GTK_MAJOR_VERSION >= 4
    gtk_editable_set_text(GTK_EDITABLE(entry), text);
#else
    gtk_entry_set_text(entry, text);
#endif
}

static inline char *srn_gtk_file_chooser_get_filename(GtkFileChooser *chooser){
#if GTK_MAJOR_VERSION >= 4
    GFile *file;
    char *path;

    file = gtk_file_chooser_get_file(chooser);
    if (!file){
        return NULL;
    }

    path = g_file_get_path(file);
    g_object_unref(file);

    return path;
#else
    return gtk_file_chooser_get_filename(chooser);
#endif
}

static inline void srn_gtk_file_chooser_set_filename(GtkFileChooser *chooser,
        const char *filename){
#if GTK_MAJOR_VERSION >= 4
    GFile *file;

    if (filename == NULL || filename[0] == '\0'){
        gtk_file_chooser_set_file(chooser, NULL, NULL);
        return;
    }

    file = g_file_new_for_path(filename);
    gtk_file_chooser_set_file(chooser, file, NULL);
    g_object_unref(file);
#else
    gtk_file_chooser_set_filename(chooser, filename);
#endif
}

static inline char *srn_gtk_file_selector_get_filename(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    if (GTK_IS_EDITABLE(widget)){
        const char *text;

        text = gtk_editable_get_text(GTK_EDITABLE(widget));
        if (text == NULL || text[0] == '\0'){
            return NULL;
        }
        return g_strdup(text);
    }
#endif
    return srn_gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
}

static inline void srn_gtk_file_selector_set_filename(GtkWidget *widget,
        const char *filename){
#if GTK_MAJOR_VERSION >= 4
    if (GTK_IS_EDITABLE(widget)){
        gtk_editable_set_text(GTK_EDITABLE(widget), filename ? filename : "");
        return;
    }
#endif
    srn_gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget), filename);
}

static inline void srn_gtk_box_pack_start(GtkBox *box, GtkWidget *child,
        gboolean expand, gboolean fill, guint padding){
#if GTK_MAJOR_VERSION >= 4
    gtk_box_append(box, child);
#else
    gtk_box_pack_start(box, child, expand, fill, padding);
#endif
}

static inline void srn_gtk_box_pack_end(GtkBox *box, GtkWidget *child,
        gboolean expand, gboolean fill, guint padding){
#if GTK_MAJOR_VERSION >= 4
    gtk_box_append(box, child);
#else
    gtk_box_pack_end(box, child, expand, fill, padding);
#endif
}

static inline void srn_gtk_button_clicked(GtkButton *button){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_activate(GTK_WIDGET(button));
#else
    gtk_button_clicked(button);
#endif
}

static inline GtkWindow *srn_gtk_widget_get_window_root(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    return GTK_WINDOW(gtk_widget_get_root(widget));
#else
    return GTK_WINDOW(gtk_widget_get_toplevel(widget));
#endif
}

static inline GdkSurface *srn_gtk_widget_get_surface(GtkWidget *widget){
#if GTK_MAJOR_VERSION >= 4
    GtkRoot *root;

    root = gtk_widget_get_root(widget);
    if (!GTK_IS_NATIVE(root)){
        return NULL;
    }
    return gtk_native_get_surface(GTK_NATIVE(root));
#else
    return gtk_widget_get_window(widget);
#endif
}

static inline void srn_gtk_process_pending_events(void){
#if GTK_MAJOR_VERSION >= 4
    while (g_main_context_pending(NULL)){
        g_main_context_iteration(NULL, FALSE);
    }
#else
    while (gtk_events_pending()){
        gtk_main_iteration();
    }
#endif
}

typedef struct _SrnGtkDialogRunData {
    GMainLoop *loop;
    int response_id;
} SrnGtkDialogRunData;

static inline void srn_gtk_dialog_on_response(GtkDialog *dialog,
        int response_id, gpointer user_data){
    SrnGtkDialogRunData *data;

    data = user_data;
    data->response_id = response_id;
    g_main_loop_quit(data->loop);
}

static inline int srn_gtk_dialog_run(GtkDialog *dialog){
#if GTK_MAJOR_VERSION >= 4
    gulong handler_id;
    SrnGtkDialogRunData data;

    data.loop = g_main_loop_new(NULL, FALSE);
    data.response_id = GTK_RESPONSE_NONE;

    handler_id = g_signal_connect(dialog, "response",
            G_CALLBACK(srn_gtk_dialog_on_response), &data);
    gtk_window_present(GTK_WINDOW(dialog));
    g_main_loop_run(data.loop);
    g_signal_handler_disconnect(dialog, handler_id);
    g_main_loop_unref(data.loop);

    return data.response_id;
#else
    return gtk_dialog_run(dialog);
#endif
}

static inline void srn_gtk_header_bar_set_show_close_button(
        GtkHeaderBar *header_bar, gboolean visible){
#if GTK_MAJOR_VERSION >= 4
    gtk_header_bar_set_show_title_buttons(header_bar, visible);
#else
    gtk_header_bar_set_show_close_button(header_bar, visible);
#endif
}

static inline void srn_gtk_header_bar_set_title(GtkHeaderBar *header_bar,
        const char *title){
#if GTK_MAJOR_VERSION >= 4
    GtkWidget *label;

    label = gtk_header_bar_get_title_widget(header_bar);
    if (title == NULL){
        gtk_header_bar_set_title_widget(header_bar, NULL);
        return;
    }
    if (!GTK_IS_LABEL(label)){
        label = gtk_label_new(title);
        gtk_header_bar_set_title_widget(header_bar, label);
    } else {
        gtk_label_set_text(GTK_LABEL(label), title);
    }
#else
    gtk_header_bar_set_title(header_bar, title);
#endif
}

static inline void srn_gtk_header_bar_set_custom_title(
        GtkHeaderBar *header_bar, GtkWidget *title_widget){
#if GTK_MAJOR_VERSION >= 4
    gtk_header_bar_set_title_widget(header_bar, title_widget);
#else
    gtk_header_bar_set_custom_title(header_bar, title_widget);
#endif
}

static inline void srn_gtk_menu_button_set_popup(GtkMenuButton *button,
        GtkWidget *popup){
#if GTK_MAJOR_VERSION >= 4
    GtkPopover *popover;

    if (GTK_IS_POPOVER(popup)){
        popover = GTK_POPOVER(popup);
    } else {
        popover = GTK_POPOVER(gtk_popover_new());
        gtk_popover_set_child(popover, popup);
    }
    gtk_menu_button_set_popover(button, GTK_WIDGET(popover));
#else
    gtk_menu_button_set_popup(button, popup);
#endif
}

#endif /* __GTK_COMPAT_H */
