/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
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

/**
 * @file sui_common.c
 * @brief useful functions required by UI module
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <string.h>
#include <gtk/gtk.h>
#include <assert.h>

#include "i18n.h"
#include "log.h"

#include "sui_event_hdr.h"
#include "sui_common.h"
#include "sui_theme.h"
#include "sui_app.h"
#include "sui_chat_buffer.h"
#include "sui_url_previewer.h"

static void popover_on_hide(GtkWidget *widget, gpointer user_data);

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

/**
 * @brief used to create an unfocusable listbox row
 * @param listbox
 * @param widget
 *
 * @return a unfocusable GtkListRow
 */

GtkListBoxRow* sui_common_unfocusable_list_box_row_new(GtkWidget *widget){
    GtkListBoxRow *row;

    row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
    gtk_widget_set_can_focus(GTK_WIDGET(row), FALSE);
    gtk_container_add(GTK_CONTAINER(row), widget);
    gtk_widget_show(GTK_WIDGET(row));
    gtk_widget_show(GTK_WIDGET(widget));

    return row;
}

void sui_common_scale_size(int src_width, int src_height,
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

/**
 * @brief sui_common_activate_gtk_label_link General "activate-link" signal callback
 *
 * @param label
 * @param url
 * @param user_data
 *
 * @return
 */
gboolean sui_common_activate_gtk_label_link(GtkLabel *label, const char *url, gpointer user_data){
    return RET_IS_OK(sui_common_open_url(url));
}

SrnRet sui_common_open_url(const char *url){
    int event_time;
    const char *urls[]  = {url, NULL};
    GError *err;
    GVariantDict *params;
    SrnRet ret;
    SuiApplication *app;

    app = sui_application_get_instance();
    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "urls", SUI_EVENT_PARAM_STRINGS, urls, -1);

    ret = sui_application_event_hdr(app, SUI_EVENT_OPEN, params);
    if (RET_IS_OK(ret)){
        goto FIN;
    }

    event_time = gtk_get_current_event_time();
    err = NULL;
#if GTK_CHECK_VERSION(3, 22, 0)
    gtk_show_uri_on_window(GTK_WINDOW(sui_common_get_cur_window()), url, event_time, &err);
#else
    gtk_show_uri(NULL, url, event_time, &err);
#endif
    if (err) {
        ret = RET_ERR(_("Failed to open URL \"%1$s\": %2$s"), url, err->message);
        sui_message_box(_("Error"), RET_MSG(ret));
        g_error_free(err);
    } else {
        ret = SRN_OK;
    }

FIN:
    g_variant_dict_unref(params);
    return ret;
}

SuiWindow *sui_common_get_cur_window(){
    SuiApplication *app;
    SuiWindow *win;

    app = sui_application_get_instance();
    g_return_val_if_fail(app, NULL);
    win = sui_application_get_cur_window(app);
    g_return_val_if_fail(win, NULL);

    return win;
}

SuiBuffer *sui_common_get_cur_buffer(){
    SuiWindow *win;
    SuiBuffer *buf;

    win = sui_common_get_cur_window();
    buf = sui_window_get_cur_buffer(win);
    g_return_val_if_fail(buf, NULL);

    return buf;
}

SuiServerBuffer *sui_common_get_cur_server_buffer(){
    SuiBuffer *buf;

    buf = sui_common_get_cur_buffer();
    g_return_val_if_fail(buf, NULL);

    if (SUI_IS_SERVER_BUFFER(buf)){
        return SUI_SERVER_BUFFER(buf);
    } else if (SUI_IS_CHAT_BUFFER(buf)){
        return sui_chat_buffer_get_server_buffer(SUI_CHAT_BUFFER(buf));
    }

    g_return_val_if_reached(NULL);
}

/**
 * @brief Popdown the GtkPopover create by ``sui_common_popup_panel``.
 *
 * @param child
 */
void sui_common_popdown_panel(GtkWidget *child){
    GtkWidget *parent;

    parent = gtk_widget_get_parent(child);
    if (GTK_IS_POPOVER(parent)){
        gtk_popover_popdown(GTK_POPOVER(parent));
    }
}

/**
 * @brief Create and popup a GtkPopover to show the widget ``child``.
 * The created GtkPopover will be freed after it popdown.
 *
 * @param relative_to
 * @param child
 */
void sui_common_popup_panel(GtkWidget *relative_to, GtkWidget *child){
    GtkPopover *popover;
    GtkStyleContext *style_context;

    popover = GTK_POPOVER(gtk_popover_new());
    gtk_popover_set_relative_to(popover, relative_to);
    gtk_container_add(GTK_CONTAINER(popover), child);

    style_context = gtk_widget_get_style_context(GTK_WIDGET(popover));
    gtk_style_context_add_class(style_context, "sui-panel");

    g_signal_connect(popover, "hide",
            G_CALLBACK(popover_on_hide), NULL);

    gtk_popover_popup(popover);
}

void sui_common_popup_panel_at_point(GtkWidget *relative_to, GtkWidget *child,
        int x, int y){
    GdkRectangle rect;
    GtkPopover *popover;

    rect.x = x;
    rect.y = y;
    rect.width = rect.height = 1;

    popover = GTK_POPOVER(gtk_popover_new());
    gtk_popover_set_relative_to(popover, relative_to);
    gtk_popover_set_pointing_to(popover, &rect);
    gtk_container_add(GTK_CONTAINER(popover), child);
    gtk_container_set_border_width(GTK_CONTAINER(popover), 6);

    g_signal_connect(popover, "hide",
            G_CALLBACK(popover_on_hide), NULL);

    gtk_popover_popup(popover);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void popover_on_hide(GtkWidget *widget, gpointer user_data){
    GtkWidget *child;
    GtkPopover *popover;

    popover = GTK_POPOVER(widget);
    child = gtk_bin_get_child(GTK_BIN(popover));
    gtk_container_remove(GTK_CONTAINER(popover), child);
    g_object_unref(popover); // Free popover itself
}
