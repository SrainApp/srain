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
 * @file sui_message_list.c
 * @brief A auto-scrolling, dynamic loading listbox used to display messages
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-05-19
 *
 * TODO: Smooth scroll support.
 */

#include <gtk/gtk.h>
#include <string.h>

#include "sui_common.h"
#include "sui_window.h"
#include "sui_message_list.h"

#include "i18n.h"
#include "log.h"

struct _SuiMessageList {
    GtkBox parent;

    int scroll_timer;
    GtkScrolledWindow *scrolled_window;
    GtkViewport *viewport;
    GtkListBox *list_box;

    GtkBox *browse_box; // Box for the go_xxx buttons
    GtkButton *go_prev_mention_button;
    GtkButton *go_next_mention_button;
    GtkButton *go_bottom_button;

    SuiMessage *first_msg; // Current first message
    GtkListBoxRow *first_row; // Container of first_msg
    SuiMessage *last_msg; // Current last message
    GtkListBoxRow *last_row; // Container of last_msg
};

struct _SuiMessageListClass {
    GtkBoxClass parent_class;
};

static void scroll_to_bottom(SuiMessageList *self);
static gboolean scroll_to_bottom_timeout(gpointer user_data);
static void smart_scroll(SuiMessageList *self);
static double get_page_count_to_bottom(SuiMessageList *self);

static void scrolled_window_on_edge_reached(GtkScrolledWindow *swin,
               GtkPositionType pos, gpointer user_data);
static void scrolled_window_on_edge_overshot(GtkScrolledWindow *swin,
        GtkPositionType pos, gpointer user_data);
static void scrolled_window_vadjustment_on_value_changed(
        GtkAdjustment *adjustment, gpointer user_data);
static void go_prev_mention_button_on_click(GtkButton *button, gpointer user_data);
static void go_next_mention_button_on_click(GtkButton *button, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiMessageList, sui_message_list, GTK_TYPE_BOX);

static void sui_message_list_init(SuiMessageList *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(self->scrolled_window, "edge-overshot",
            G_CALLBACK(scrolled_window_on_edge_overshot), self);
    g_signal_connect(self->scrolled_window, "edge-reached",
            G_CALLBACK(scrolled_window_on_edge_reached), self);
    g_signal_connect(self->go_prev_mention_button, "clicked",
            G_CALLBACK(go_prev_mention_button_on_click), self);
    g_signal_connect(self->go_next_mention_button, "clicked",
            G_CALLBACK(go_next_mention_button_on_click), self);
    g_signal_connect_swapped(self->go_bottom_button, "clicked",
            G_CALLBACK(scroll_to_bottom), self);
    g_signal_connect(
            gtk_scrolled_window_get_vadjustment(self->scrolled_window),
            "value-changed",
            G_CALLBACK(scrolled_window_vadjustment_on_value_changed), self);

    g_object_bind_property(
            gtk_scrolled_window_get_vscrollbar(self->scrolled_window),
            "opacity",
            self->browse_box,
            "opacity",
            0);

    // Tell GtkScrolledWindow scrolls to show a row of GtkListBox when it is
    // focused. It is required by gtk_container_set_focus_child().
    gtk_container_set_focus_vadjustment(GTK_CONTAINER(self->list_box),
            gtk_scrolled_window_get_vadjustment(self->scrolled_window));
}

static void sui_message_list_finalize(GObject *object){
    SuiMessageList *self;

    self = SUI_MESSAGE_LIST(object);
    if (self->scroll_timer) {
        g_source_remove(self->scroll_timer);
    }

    G_OBJECT_CLASS(sui_message_list_parent_class)->finalize(object);
}

static void sui_message_list_class_init(SuiMessageListClass *class){
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;


    object_class = G_OBJECT_CLASS(class);
    object_class->finalize = sui_message_list_finalize;

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/message_list.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, scrolled_window);
    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, viewport);
    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, list_box);
    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, browse_box);
    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, go_prev_mention_button);
    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, go_next_mention_button);
    gtk_widget_class_bind_template_child(widget_class, SuiMessageList, go_bottom_button);
}


/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiMessageList* sui_message_list_new(void){
    return g_object_new(SUI_TYPE_MESSAGE_LIST, NULL);
}

void sui_message_list_scroll_up(SuiMessageList *self, double step){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(self->scrolled_window);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) - step);
}

void sui_message_list_scroll_down(SuiMessageList *self, double step){
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(self->scrolled_window);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_value(adj) + step);
}

void sui_message_list_append_message(SuiMessageList *self, SuiMessage *msg,
        GtkAlign halign){
    GtkBox *box;
    GtkListBoxRow *row;

    if (self->last_msg
            && (G_OBJECT_TYPE(msg) == G_OBJECT_TYPE(self->last_msg))) {
        sui_message_compose_next(self->last_msg, msg);
        sui_message_compose_prev(msg, self->last_msg);
    }
    self->last_msg = msg;

    box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start(box, GTK_WIDGET(msg), TRUE, TRUE, 0);
    gtk_widget_set_halign(GTK_WIDGET(msg), halign);
    row = sui_common_unfocusable_list_box_row_new(GTK_WIDGET(box));
    gtk_list_box_insert(self->list_box, GTK_WIDGET(row), -1);
    self->last_row = row;

    smart_scroll(self);
}

void sui_message_list_prepend_message(SuiMessageList *self, SuiMessage *msg,
        GtkAlign halign){
    GtkBox *box;
    GtkListBoxRow *row;

    if (self->first_msg
            && (G_OBJECT_TYPE(msg) == G_OBJECT_TYPE(self->first_msg))) {
        sui_message_compose_prev(self->first_msg, msg);
        sui_message_compose_next(msg, self->first_msg);
    }
    self->first_msg = msg;

    box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start(box, GTK_WIDGET(msg), TRUE, TRUE, 0);
    gtk_widget_set_halign(GTK_WIDGET(msg), halign);
    row = sui_common_unfocusable_list_box_row_new(GTK_WIDGET(box));
    gtk_list_box_prepend(self->list_box, GTK_WIDGET(row));
    self->first_row = row;

    smart_scroll(self);
}

void sui_message_list_add_message(SuiMessageList *self, SuiMessage *msg,
        GtkAlign halign){
    sui_message_list_append_message(self, msg, halign);
}

GList *sui_message_list_get_recent_messages(SuiMessageList *self, int limit){
    GList *rows;
    GList *lst;
    GList *msgs;

    rows = gtk_container_get_children(GTK_CONTAINER(self->list_box));
    lst = g_list_last(rows);
    msgs = NULL;

    while (lst && limit){
        GtkListBoxRow *row;
        SuiMessage *msg;
        GtkBox *box;
        GList *box_child;

        row = GTK_LIST_BOX_ROW(lst->data);
        box = GTK_BOX(gtk_bin_get_child(GTK_BIN(row)));
        box_child = gtk_container_get_children(GTK_CONTAINER(box));
        g_return_val_if_fail(box_child, NULL);

        msg = SUI_MESSAGE(box_child->data);
        msgs = g_list_append(msgs, msg);

        g_list_free(box_child);

        lst = g_list_previous(lst);
        limit--;
    }
    g_list_free(rows);

    return msgs;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void scroll_to_bottom(SuiMessageList *self){
    if (self->scroll_timer){
        return;
    }
    // Use timer for avoiding duplicated calls of this function.
    // And the allocated size of the ``SuiMessage`` may changed after being
    // added into list. So do scroll later.
    self->scroll_timer = g_timeout_add(100, scroll_to_bottom_timeout, self);
}

static gboolean scroll_to_bottom_timeout(gpointer user_data){
    GtkAdjustment *adj;
    SuiMessageList *self;

    self = SUI_MESSAGE_LIST(user_data);

    /* Scroll to bottom */
    adj = gtk_scrolled_window_get_vadjustment(self->scrolled_window);
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) -
            gtk_adjustment_get_page_size(adj));

    self->scroll_timer = 0;

    return G_SOURCE_REMOVE;
}

/**
 * @brief ``smart_scroll`` scrolls a ``SuiMessageList`` to bottom according to
 * the current situation(see below).
 *
 * @param self
 *
 * This function called when a message added to ``SuiMessageList``.
 * If:
 * - The top-level window is visiable;
 * - And the SuiMessageList itself is child of current ``SuiBuffer``;
 * - The scroll bar is near the bottom of the list box
 *   (If not, the user may be browsing previous messages).
 * Scroll the the scrolled window to bottom by calling ``scroll_to_bottom()``.
 *
 */
static void smart_scroll(SuiMessageList *self){
    SuiWindow *win;
    SuiBuffer *buf;

    win = sui_common_get_cur_window();
    g_return_if_fail(SUI_IS_WINDOW(win));
    buf = sui_window_get_cur_buffer(win);
    g_return_if_fail(SUI_IS_BUFFER(buf));

    if (sui_buffer_get_message_list(buf) != self){
        return;
    }
    if (!gtk_widget_get_visible(GTK_WIDGET(win))){
        return;
    }

    // 0.15 page is the threshold for judging whether "the scroll bar is very
    // near to the bottom of the list"
    if (get_page_count_to_bottom(self) > 0.15){
        return;
    }

    scroll_to_bottom(self);
}

/* ``scrolled_window_on_edge_overshot()`` and ``scrolled_window_on_edge_reached()``
 * are used for implement dynamic hide&load messages */

static void scrolled_window_on_edge_overshot(GtkScrolledWindow *swin,
        GtkPositionType pos, gpointer user_data){
    switch (pos) {
        case GTK_POS_TOP:
            // TODO: Dynamic load
            break;
        case GTK_POS_BOTTOM:
            break;
        default:
            break;
    }
}

static void scrolled_window_on_edge_reached(GtkScrolledWindow *swin,
               GtkPositionType pos, gpointer user_data){
    SuiMessageList *self;

    self = SUI_MESSAGE_LIST(user_data);

    switch (pos) {
        case GTK_POS_TOP:
            break;
        case GTK_POS_BOTTOM:
            // TODO: Dynamic free
            gtk_widget_hide(GTK_WIDGET(self->browse_box));
            break;
        default:
            break;
    }
}

// Get the count of pages from current position to the bottom of message list.
static double get_page_count_to_bottom(SuiMessageList *self) {
    double val;
    double max_val;
    double page_size;
    GtkAdjustment *adj;

    adj = gtk_scrolled_window_get_vadjustment(self->scrolled_window);
    page_size = gtk_adjustment_get_page_size(adj);
    val = gtk_adjustment_get_value(adj);
    max_val = gtk_adjustment_get_upper(adj) - page_size;

    if (page_size == 0) {
        return 0;
    }
    return (max_val - val) / page_size;
}

static void scrolled_window_vadjustment_on_value_changed(
        GtkAdjustment *adj, gpointer user_data){
    SuiMessageList *self;

    self = SUI_MESSAGE_LIST((user_data));

    // The go bottom button appears each time the distance from current
    // position to bottom is greater than 0.5 page (we think user is browsing
    // message now).
    gtk_widget_set_visible(GTK_WIDGET(self->browse_box),
            get_page_count_to_bottom(self) > 0.5);
}

static void go_prev_mention_button_on_click(GtkButton *button, gpointer user_data) {
    SuiMessageList *self;
    GtkListBoxRow *row;

    self = user_data;
    // Starts from selected row
    row = gtk_list_box_get_selected_row(self->list_box);
    if (!row) {
        // No row selected, use last row
        row = self->last_row;
    }
    if (!row) {
        // No any row in list
        return;
    }

    // Fine prev mentioned message
    for (int i = gtk_list_box_row_get_index(row); i > 0; i--) {
        GtkContainer *box;
        GList *lst;
        SuiMessage *msg;

        row = gtk_list_box_get_row_at_index(self->list_box, i);
        box = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(row)));
        lst = gtk_container_get_children(box);
        msg = lst->data;
        g_list_free(lst);
        if (!sui_message_is_mentioned(msg)) {
            continue;
        }

        // Focus and select
        gtk_list_box_select_row(self->list_box, row);
        gtk_container_set_focus_child(GTK_CONTAINER(self->list_box), GTK_WIDGET(row));
    }
}

static void go_next_mention_button_on_click(GtkButton *button, gpointer user_data) {
    SuiMessageList *self;
    GtkListBoxRow *row;

    self = user_data;
    // Starts from selected row
    row = gtk_list_box_get_selected_row(self->list_box);
    if (!row) {
        // No row selected, use last row
        row = self->last_row;
    }
    if (!row) {
        // No any row in list
        return;
    }

    // Fine next mentioned message
    for (int i = gtk_list_box_row_get_index(row);
            gtk_list_box_get_row_at_index(self->list_box, i) != NULL; i++) {
        GtkContainer *box;
        SuiMessage *msg;
        GList *lst;

        row = gtk_list_box_get_row_at_index(self->list_box, i);
        box = GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(row)));
        lst = gtk_container_get_children(box);
        msg = lst->data;
        g_list_free(lst);
        if (!sui_message_is_mentioned(msg)) {
            continue;
        }

        // Focus and select
        gtk_list_box_select_row(self->list_box, row);
        gtk_container_set_focus_child(GTK_CONTAINER(self->list_box), GTK_WIDGET(row));
    }
}
