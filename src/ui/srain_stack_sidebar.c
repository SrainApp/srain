/**
 * @file srain_stack_sidebar.c
 * @brief a simplified, customized stacksidebar implement
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-06
 *
 * a simplified, customized stacksidebar implement, modified from
 * <git.gnome.org/browse/gtk+/tree/gtk/gtkstacksidebar.c>
 */

/*
 * Copyright(c) 2014 Intel Corporation
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

#define __LOG_ON

#include <gtk/gtk.h>
#include "srain_stack_sidebar.h"
#include "srain_stack_sidebar_item.h"
#include "srain_chan.h"
#include "log.h"

struct _SrainStackSidebar {
    GtkBin parent;
    GtkListBox *list;
    GtkStack *stack;
    GHashTable *rows;
};

struct _SrainStackSidebarClass {
    GtkBinClass parent_class;
};


G_DEFINE_TYPE(SrainStackSidebar, srain_stack_sidebar, GTK_TYPE_BIN)

static void srain_stack_sidebar_row_selected(GtkListBox *box, GtkListBoxRow *row, gpointer user_data){
    SrainStackSidebar *sidebar;
    SrainStackSidebarItem *item;
    GtkWidget *child;

    sidebar = SRAIN_STACK_SIDEBAR(user_data); 

    if (!row) return;

    item = SRAIN_STACK_SIDEBAR_ITEM(gtk_bin_get_child(GTK_BIN(row)));
    child = g_object_get_data(G_OBJECT(item), "stack-child");
    gtk_stack_set_visible_child(sidebar->stack, child);

    srain_stack_sidebar_item_count_clear(item);
    srain_chan_fcous_entry(SRAIN_CHAN(child));
}

static void srain_stack_sidebar_init(SrainStackSidebar *self){
    GtkWidget *sw;
    GtkStyleContext *style;

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(sw);
    gtk_widget_set_no_show_all(sw, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(self), sw);

    self->list = GTK_LIST_BOX(gtk_list_box_new());
    gtk_widget_show(GTK_WIDGET(self->list));

    gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(self->list));

    g_signal_connect(self->list, "row-selected", G_CALLBACK(srain_stack_sidebar_row_selected), self);

    style = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(style, "sidebar"); // ?

    self->rows = g_hash_table_new(NULL, NULL);
}

static void add_child(GtkWidget *child, SrainStackSidebar *sidebar){
    const char *name;
    SrainStackSidebarItem *item;
    GtkWidget *row;

    if (g_hash_table_lookup(sidebar->rows, child))
        return;

    // TODO
    name = gtk_widget_get_name(child);
    item = srain_stack_sidebar_item_new(name);
    // gtk_widget_set_halign(item, GTK_ALIGN_START);
    // gtk_widget_set_valign(item, GTK_ALIGN_CENTER);
    row = gtk_list_box_row_new();
    gtk_widget_set_can_focus(row, FALSE);
    gtk_container_add(GTK_CONTAINER(row), GTK_WIDGET(item));
    gtk_widget_show_all(row);

    g_object_set_data(G_OBJECT(item), "stack-child", child);
    g_hash_table_insert(sidebar->rows, child, row);
    gtk_container_add(GTK_CONTAINER(sidebar->list), row);

    gtk_list_box_select_row(sidebar->list, GTK_LIST_BOX_ROW(row));
}

static void remove_child(GtkWidget *widget, SrainStackSidebar *sidebar){
    GtkWidget *row;

    row = g_hash_table_lookup(sidebar->rows, widget);
    if (!row) return;

    gtk_container_remove(GTK_CONTAINER(sidebar->list), row);
    g_hash_table_remove(sidebar->rows, widget);
}

static void populate_sidebar(SrainStackSidebar *sidebar){
    GtkWidget *widget, *row;

    gtk_container_foreach(GTK_CONTAINER(sidebar->stack),(GtkCallback)add_child, sidebar);

    widget = gtk_stack_get_visible_child(sidebar->stack);
    if (widget){
        row = g_hash_table_lookup(sidebar->rows, widget);
        gtk_list_box_select_row(sidebar->list, GTK_LIST_BOX_ROW(row));
    }
}

static void clear_sidebar(SrainStackSidebar *sidebar){
    gtk_container_foreach(GTK_CONTAINER(sidebar->stack), (GtkCallback)remove_child, sidebar);
}

static void on_child_changed(GtkWidget *widget, GParamSpec *pspec, SrainStackSidebar *sidebar){
    GtkWidget *child;
    GtkWidget *row;

    child = gtk_stack_get_visible_child(GTK_STACK(widget));
    row = g_hash_table_lookup(sidebar->rows, child);
    if (row != NULL) {
        gtk_list_box_select_row(sidebar->list, GTK_LIST_BOX_ROW(row));
    }
}

static void on_stack_child_added(GtkContainer *container, GtkWidget *widget, SrainStackSidebar *sidebar){
    add_child(widget, sidebar);
}

static void on_stack_child_removed(GtkContainer *container, GtkWidget *widget, SrainStackSidebar *sidebar){
    remove_child(widget, sidebar);
}

static void disconnect_stack_signals(SrainStackSidebar *sidebar){
    g_signal_handlers_disconnect_by_func(sidebar->stack, on_stack_child_added, sidebar);
    g_signal_handlers_disconnect_by_func(sidebar->stack, on_stack_child_removed, sidebar);
    g_signal_handlers_disconnect_by_func(sidebar->stack, on_child_changed, sidebar);
    g_signal_handlers_disconnect_by_func(sidebar->stack, disconnect_stack_signals, sidebar);
}

static void connect_stack_signals(SrainStackSidebar *sidebar){
    g_signal_connect_after(sidebar->stack, "add", G_CALLBACK(on_stack_child_added), sidebar);
    g_signal_connect_after(sidebar->stack, "remove", G_CALLBACK(on_stack_child_removed), sidebar);
    g_signal_connect(sidebar->stack, "notify::visible-child", G_CALLBACK(on_child_changed), sidebar);
    g_signal_connect_swapped(sidebar->stack, "destroy", G_CALLBACK(disconnect_stack_signals), sidebar);
}

static void srain_stack_sidebar_finalize(GObject *object){
    SrainStackSidebar *sidebar = SRAIN_STACK_SIDEBAR(object);

    g_hash_table_destroy(sidebar->rows);

    G_OBJECT_CLASS(srain_stack_sidebar_parent_class)->finalize(object);
}

static void srain_stack_sidebar_class_init(SrainStackSidebarClass *klass){
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = srain_stack_sidebar_finalize;
}

SrainStackSidebar *srain_stack_sidebar_new(){
    return g_object_new(SRAIN_TYPE_STACK_SIDEBAR, NULL);
}

void srain_stack_sidebar_set_stack(SrainStackSidebar *sidebar, GtkStack *stack)
{
    g_return_if_fail(SRAIN_IS_STACK_SIDEBAR(sidebar));
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

GtkStack *srain_stack_sidebar_get_stack(SrainStackSidebar *sidebar){
    g_return_val_if_fail(GTK_IS_STACK_SIDEBAR(sidebar), NULL);

    return GTK_STACK(sidebar->stack);
}

void srain_stack_sidebar_update(SrainStackSidebar *sidebar, SrainChan *chan, const char *nick, const char *msg, int is_visible){
    GtkListBoxRow *row;
    SrainStackSidebarItem *item;

    row = g_hash_table_lookup(sidebar->rows, chan);
    if (!row) return;

    item = SRAIN_STACK_SIDEBAR_ITEM(gtk_bin_get_child(GTK_BIN(row)));
    srain_stack_sidebar_item_recentmsg_update(item, nick, msg);

    if (!is_visible){
        srain_stack_sidebar_item_count_inc(item);
    }
}

void srain_stack_sidebar_prev(SrainStackSidebar *sidebar){
    GList *rows;
    GtkListBoxRow *tail, *cur_row;

    cur_row = gtk_list_box_get_selected_row(sidebar->list);

    rows = gtk_container_get_children(GTK_CONTAINER(sidebar->list));

    while (rows){
        if (rows->data == cur_row){
            if (rows->prev){
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(rows->prev->data));
            } else {
                while (rows->next){
                    rows = rows->next;
                }
                tail = rows->data;
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(tail));
                return;
            }
        }
        rows = rows->next;
    }
}

void srain_stack_sidebar_next(SrainStackSidebar *sidebar){
    GList *rows;
    GtkListBoxRow *head, *cur_row;

    cur_row = gtk_list_box_get_selected_row(sidebar->list);

    rows = gtk_container_get_children(GTK_CONTAINER(sidebar->list));
    if (rows){
        head = GTK_LIST_BOX_ROW(rows->data);
    }

    while (rows){
        if (rows->data == cur_row){
            if (rows->next){
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(rows->next->data));
            } else {
                gtk_list_box_select_row(sidebar->list,
                        GTK_LIST_BOX_ROW(head));
            }
        }
        rows = rows->next;
    }
}
