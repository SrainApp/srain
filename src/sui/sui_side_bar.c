/*
 * Copyright (C) 2014 Intel Corporation
 * Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or(at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author:
 *      Ikey Doherty <michael.i.doherty@intel.com>
 */

/**
 * @file sui_side_bar.c
 * @brief A simplified, customized stacksidebar implementation
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-06
 *
 * Originated from <git.gnome.org/browse/gtk+/tree/gtk/gtkstacksidebar.c>.
 */

#include <gtk/gtk.h>

#include "core/core.h"

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_side_bar.h"

#include "log.h"

struct _SuiSideBar {
    GtkBin parent;
    GtkListBox *list;
    GtkStack *stack;
    GHashTable *rows;
};

struct _SuiSideBarClass {
    GtkBinClass parent_class;
};


G_DEFINE_TYPE(SuiSideBar, sui_side_bar, GTK_TYPE_BIN)

static void
listbox_on_row_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data){
    SuiSideBar *sidebar;
    SuiSideBarItem *item;
    GtkWidget *child, *event_box;

    sidebar = SUI_SIDE_BAR(user_data);

    if (!row) return;

    event_box = gtk_bin_get_child(GTK_BIN(row));
    item = SUI_SIDE_BAR_ITEM(gtk_bin_get_child(GTK_BIN(event_box)));
    child = g_object_get_data(G_OBJECT(item), "stack-child");
    gtk_stack_set_visible_child(sidebar->stack, child);

    sui_side_bar_item_clear_count(item);
}

static gboolean
list_box_on_popup(GtkWidget *widget, GdkEventButton *event, gpointer user_data){
    /* widget is the GtkEventBox containing the SuiSideBarItem */
    SuiBuffer *child;
    SuiSideBarItem *item;

    if (event->button == 3){
        item = SUI_SIDE_BAR_ITEM(gtk_bin_get_child(GTK_BIN(widget)));
        child = g_object_get_data(G_OBJECT(item), "stack-child");

        gtk_menu_popup_at_pointer(sui_buffer_get_menu(child), (GdkEvent *)event);

        return TRUE;
    }
    return FALSE;
}

static gint list_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2,
        gpointer user_data){
    unsigned long time1;
    unsigned long time2;
    SuiSideBarItem *item1;
    SuiSideBarItem *item2;
    GtkWidget *event_box;
    SuiWindowConfig* cfg;

    event_box = gtk_bin_get_child(GTK_BIN(row1));
    g_return_val_if_fail(GTK_IS_EVENT_BOX(event_box), 0);
    item1 = SUI_SIDE_BAR_ITEM(gtk_bin_get_child(GTK_BIN(event_box)));

    event_box = gtk_bin_get_child(GTK_BIN(row2));
    g_return_val_if_fail(GTK_IS_EVENT_BOX(event_box), 0);
    item2 = SUI_SIDE_BAR_ITEM(gtk_bin_get_child(GTK_BIN(event_box)));

    cfg = sui_window_get_config(sui_common_get_cur_window());
    if (g_strcmp0(cfg->chat_list_order, CHAT_LIST_ORDER_ALPHABET) == 0){
        return g_strcmp0(
                sui_side_bar_item_get_title(item1),
                sui_side_bar_item_get_title(item2)
                );
    }

    time1 = sui_side_bar_item_get_update_time(item1);
    time2 = sui_side_bar_item_get_update_time(item2);

    return time1 <= time2;
}

static gboolean list_filter_func(GtkListBoxRow *row, gpointer user_data){
    GtkWidget *event_box = gtk_bin_get_child(GTK_BIN(row));
    SuiSideBarItem *item = SUI_SIDE_BAR_ITEM(gtk_bin_get_child(GTK_BIN(event_box)));
    return gtk_widget_get_visible(GTK_WIDGET(item));
}

static void
sui_side_bar_init(SuiSideBar *self){
    GtkWidget *sw;
    GtkStyleContext *style;

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(sw);
    gtk_widget_set_no_show_all(sw, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(self), sw);

    self->list = GTK_LIST_BOX(gtk_list_box_new());
    gtk_widget_show(GTK_WIDGET(self->list));

    gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(self->list));

    gtk_list_box_set_sort_func(GTK_LIST_BOX(self->list),
            list_sort_func, NULL, NULL);
    gtk_list_box_set_filter_func(GTK_LIST_BOX(self->list),
            list_filter_func, NULL, NULL);

    g_signal_connect(self->list, "row-selected",
            G_CALLBACK(listbox_on_row_selected), self);

    style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "sidebar"); // ?

    self->rows = g_hash_table_new(NULL, NULL);
}

static void
add_child(GtkWidget *child, SuiSideBar *sidebar){
    const char *icon;
    GtkListBoxRow *row;
    SuiBuffer *buf;
    SuiSideBarItem *item;
    SrnChat *chat;
    GtkWidget *event_box;

    if (g_hash_table_lookup(sidebar->rows, child))
        return;

    buf = SUI_BUFFER(child);
    chat = sui_buffer_get_ctx(buf);

    switch (chat->type) {
        case SRN_CHAT_TYPE_SERVER:
            icon = "network-server-symbolic";
            break;
        case SRN_CHAT_TYPE_CHANNEL:
            icon = "system-users-symbolic";
            break;
        case SRN_CHAT_TYPE_DIALOG:
            icon = "user-available-symbolic";
            break;
        default:
            icon = "";
            g_warn_if_reached();
    }
    item = sui_side_bar_item_new(
            sui_buffer_get_name(buf),
            sui_buffer_get_remark(buf),
            icon);

    // gtk_widget_set_halign(item, GTK_ALIGN_START);
    // gtk_widget_set_valign(item, GTK_ALIGN_CENTER);

    /* The event_box is the actual row inserted in the list, and contains the child.
     * This allows listening for right-click events and display the contextual menu. */
    event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(event_box), GTK_WIDGET(item));

    g_object_set_data(G_OBJECT(item), "stack-child", child);

    row = sui_common_unfocusable_list_box_row_new(GTK_WIDGET(event_box));
    gtk_list_box_insert(sidebar->list, GTK_WIDGET(row), -1);
    gtk_list_box_select_row(sidebar->list, row);
    g_hash_table_insert(sidebar->rows, child, row);

    g_signal_connect(event_box, "button-press-event",
            G_CALLBACK(list_box_on_popup), NULL);

    sui_side_bar_item_update(item, NULL, "");
}

static void
remove_child(GtkWidget *widget, SuiSideBar *sidebar){
    GtkWidget *row;

    row = g_hash_table_lookup(sidebar->rows, widget);
    if (!row) return;

    gtk_container_remove(GTK_CONTAINER(sidebar->list), row);
    g_hash_table_remove(sidebar->rows, widget);
}

static void
populate_sidebar(SuiSideBar *sidebar){
    GtkWidget *widget, *row;

    gtk_container_foreach(GTK_CONTAINER(sidebar->stack),(GtkCallback)add_child, sidebar);

    widget = gtk_stack_get_visible_child(sidebar->stack);
    if (widget){
        row = g_hash_table_lookup(sidebar->rows, widget);
        gtk_list_box_select_row(sidebar->list, GTK_LIST_BOX_ROW(row));
    }
}

static void
clear_sidebar(SuiSideBar *sidebar){
    gtk_container_foreach(GTK_CONTAINER(sidebar->stack), (GtkCallback)remove_child, sidebar);
}

static void
on_child_changed(GtkWidget *widget, GParamSpec *pspec, SuiSideBar *sidebar){
    GtkWidget *child;
    GtkWidget *row;
    SuiBuffer *buf;

    child = gtk_stack_get_visible_child(GTK_STACK(widget));
    if (!child) {
        return;
    }

    g_return_if_fail(SUI_IS_BUFFER(child));
    buf = SUI_BUFFER(child);

    row = g_hash_table_lookup(sidebar->rows, child);
    if (row != NULL) {
        gtk_list_box_select_row(sidebar->list, GTK_LIST_BOX_ROW(row));
    }

    sui_buffer_event_hdr(buf, SUI_EVENT_CUTOVER, NULL);
}

static void
on_stack_child_added(GtkContainer *container, GtkWidget *widget, SuiSideBar *sidebar){
    add_child(widget, sidebar);
}

static void
on_stack_child_removed(GtkContainer *container, GtkWidget *widget, SuiSideBar *sidebar){
    remove_child(widget, sidebar);
}

static void
disconnect_stack_signals(SuiSideBar *sidebar){
    g_signal_handlers_disconnect_by_func(sidebar->stack, on_stack_child_added, sidebar);
    g_signal_handlers_disconnect_by_func(sidebar->stack, on_stack_child_removed, sidebar);
    g_signal_handlers_disconnect_by_func(sidebar->stack, on_child_changed, sidebar);
    g_signal_handlers_disconnect_by_func(sidebar->stack, disconnect_stack_signals, sidebar);
}

static void
connect_stack_signals(SuiSideBar *sidebar){
    g_signal_connect_after(sidebar->stack, "add", G_CALLBACK(on_stack_child_added), sidebar);
    g_signal_connect_after(sidebar->stack, "remove", G_CALLBACK(on_stack_child_removed), sidebar);
    g_signal_connect(sidebar->stack, "notify::visible-child", G_CALLBACK(on_child_changed), sidebar);
    g_signal_connect_swapped(sidebar->stack, "destroy", G_CALLBACK(disconnect_stack_signals), sidebar);
}

static void
sui_side_bar_dispose(GObject *object){
  SuiSideBar *sidebar = SUI_SIDE_BAR(object);

  sui_side_bar_set_stack(sidebar, NULL);

  G_OBJECT_CLASS(sui_side_bar_parent_class)->dispose(object);
}

static void
sui_side_bar_finalize(GObject *object){
    SuiSideBar *sidebar = SUI_SIDE_BAR(object);

    g_hash_table_destroy(sidebar->rows);

    G_OBJECT_CLASS(sui_side_bar_parent_class)->finalize(object);
}

static void
sui_side_bar_class_init(SuiSideBarClass *klass){
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = sui_side_bar_dispose;
    object_class->finalize = sui_side_bar_finalize;
}

SuiSideBar*
sui_side_bar_new(){
    return g_object_new(SUI_TYPE_SIDE_BAR, NULL);
}

void
sui_side_bar_set_stack(SuiSideBar *sidebar, GtkStack *stack)
{
    g_return_if_fail(SUI_IS_SIDE_BAR(sidebar));
    g_return_if_fail(GTK_IS_STACK(stack) || stack == NULL);

    if (sidebar->stack == stack) return;

    if (sidebar->stack) {
        disconnect_stack_signals(sidebar);
        clear_sidebar(sidebar);
        g_clear_object(&sidebar->stack);
    }

    if (stack) {
        sidebar->stack = g_object_ref(stack);
        populate_sidebar(sidebar);
        connect_stack_signals(sidebar);
    }

    gtk_widget_queue_resize(GTK_WIDGET(sidebar));
}

GtkStack*
sui_side_bar_get_stack(SuiSideBar *sidebar){
    g_return_val_if_fail(GTK_IS_STACK_SIDEBAR(sidebar), NULL);

    return GTK_STACK(sidebar->stack);
}

SuiSideBarItem*
sui_side_bar_get_item(SuiSideBar *sidebar, SuiBuffer *buf){
    GtkListBoxRow *row;

    row = g_hash_table_lookup(sidebar->rows, buf);
    g_return_val_if_fail(row, NULL);

    GtkWidget *event_box = gtk_bin_get_child(GTK_BIN(row));
    return SUI_SIDE_BAR_ITEM(gtk_bin_get_child(GTK_BIN(event_box)));
}

static GList *get_visible_rows(SuiSideBar* self) {
    GList *rows, *lst;

    rows = NULL;
    lst = gtk_container_get_children(GTK_CONTAINER(self->list));

    for (; lst; lst = g_list_next(lst)) {
        if (gtk_widget_get_visible(gtk_bin_get_child(GTK_BIN(lst->data)))) {
            rows = g_list_append(rows, lst->data);
        }
    }
    return rows;
}

void
sui_side_bar_prev(SuiSideBar *sidebar){
    GList *rows;
    GtkListBoxRow *tail, *cur_row;

    cur_row = gtk_list_box_get_selected_row(sidebar->list);
    rows = get_visible_rows(sidebar);

    while (rows){
        if (rows->data == cur_row){
            if (rows->prev){
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(rows->prev->data));
                break;
            } else {
                while (rows->next){
                    rows = rows->next;
                }
                tail = rows->data;
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(tail));
                break;
            }
        }
        rows = rows->next;
    }

    g_list_free(rows);
}

void sui_side_bar_next(SuiSideBar *sidebar){
    GList *rows;
    GtkListBoxRow *head, *cur_row;

    cur_row = gtk_list_box_get_selected_row(sidebar->list);
    rows = get_visible_rows(sidebar);
    if (rows){
        head = GTK_LIST_BOX_ROW(rows->data);
    }

    while (rows){
        if (rows->data == cur_row){
            if (rows->next){
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(rows->next->data));
                break;
            } else {
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(head));
                break;
            }
        }
        rows = rows->next;
    }

    g_list_free(rows);
}
