/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @file srain_window.c
 * @brief Srain's main window
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include <string.h>
#include <assert.h>

#include "sui/sui.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "theme.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_buffer.h"
#include "srain_server_buffer.h"
#include "srain_chat_buffer.h"
#include "srain_connect_popover.h"
#include "srain_join_popover.h"
#include "srain_stack_sidebar.h"
#include "tray_icon.h"

#include "meta.h"
#include "log.h"
#include "i18n.h"

struct _SrainWindow {
    GtkApplicationWindow parent;

    // Header
    GtkButton *about_button;
    GtkButton *connect_button;
    GtkButton *join_button;
    GtkSpinner *spinner;

    GtkBox *sidebar_box;
    SrainStackSidebar *sidebar;
    GtkStack *stack;
    GtkStatusIcon *tray_icon;
    GtkMenu *tray_menu;
    GtkMenuItem *about_menu_item;
    GtkMenuItem *quit_menu_item;

    // Popovers
    SrainConnectPopover *connect_popover;
    SrainJoinPopover *join_popover;
};

struct _SrainWindowClass {
    GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(SrainWindow, srain_window, GTK_TYPE_APPLICATION_WINDOW);

static void quit_menu_item_on_activate(){
    srain_app_quit(srain_app);
}

static void monitor_active_window(GObject *object, GParamSpec *pspec,
        gpointer data ) {
   if (srain_window_is_active(SRAIN_WINDOW(object))){
       /* Stop stress the icon */
       srain_window_tray_icon_stress(SRAIN_WINDOW(object), 0);
   } else {

   }
}

static void show_about_dialog(gpointer user_data){
    GtkWidget *window = user_data;
    const gchar *authors[] = { PACKAGE_AUTHOR " <" PACKAGE_EMAIL ">", NULL };
    const gchar **documentors = authors;
    const gchar *version = g_strdup_printf(_("%1$s%2$s\nRunning against GTK+ %3$d.%4$d.%5$d"),
            PACKAGE_VERSION,
            PACKAGE_BUILD,
            gtk_get_major_version(),
            gtk_get_minor_version(),
            gtk_get_micro_version());

    gtk_show_about_dialog(GTK_WINDOW(window),
            "program-name", PACKAGE_NAME,
            "version", version,
            "copyright", "(C) " PACKAGE_COPYRIGHT_DATES " " PACKAGE_AUTHOR,
            "license-type", GTK_LICENSE_GPL_3_0,
            "website", PACKAGE_WEBSITE,
            "comments", PACKAGE_DESC,
            "authors", authors,
            "documenters", documentors,
            "logo-icon-name", "srain",
            "title", _("About Srain"),
            NULL);
}

static void popover_button_on_click(gpointer user_data){
    GtkPopover *popover;

    popover = user_data;
    gtk_widget_set_visible(GTK_WIDGET(popover),
            !gtk_widget_get_visible(GTK_WIDGET(popover)));
}

static gboolean CTRL_J_K_on_press(GtkAccelGroup *group, GObject *obj,
        guint keyval, GdkModifierType mod, gpointer user_data){
    SrainStackSidebar *sidebar;

    if (mod != GDK_CONTROL_MASK) return FALSE;

    sidebar = user_data;
    switch (keyval){
        case GDK_KEY_k:
            srain_stack_sidebar_prev(sidebar);
            break;
        case GDK_KEY_j:
            srain_stack_sidebar_next(sidebar);
            break;
        default:
            ERR_FR("unknown keyval %d", keyval);
            return FALSE;
    }

    return TRUE;
}

static void srain_window_init(SrainWindow *self){
    GClosure *closure_j;
    GClosure *closure_k;
    GtkAccelGroup *accel;

    gtk_widget_init_template(GTK_WIDGET(self));

    /* Stack sidebar init */
    self->sidebar = srain_stack_sidebar_new();
    gtk_widget_show(GTK_WIDGET(self->sidebar));
    gtk_box_pack_start(self->sidebar_box, GTK_WIDGET(self->sidebar),
            TRUE, TRUE, 0);
    srain_stack_sidebar_set_stack(self->sidebar, self->stack);

    /* Popover init */
    self->connect_popover = srain_connect_popover_new(
            GTK_WIDGET(self->connect_button));
    self->join_popover = srain_join_popover_new(
            GTK_WIDGET(self->join_button));

    theme_apply(GTK_WIDGET(self));
    theme_apply(GTK_WIDGET(self->tray_menu));

    tray_icon_set_callback(self->tray_icon, self, self->tray_menu);
    g_signal_connect_swapped(self->quit_menu_item, "activate",
            G_CALLBACK(quit_menu_item_on_activate), NULL);
    g_signal_connect_swapped(self->about_menu_item, "activate",
            G_CALLBACK(show_about_dialog), self);


    g_signal_connect(self, "notify::is-active",
            G_CALLBACK(monitor_active_window), NULL);

    // Click to show/hide GtkPopover
    g_signal_connect_swapped(self->about_button, "clicked",
            G_CALLBACK(show_about_dialog), self);
    g_signal_connect_swapped(self->connect_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->connect_popover);
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->join_popover);

    /* shortcut <C-j> and <C-k> */
    accel = gtk_accel_group_new();

    closure_j = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press),
            self->sidebar, NULL);
    closure_k = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press),
            self->sidebar, NULL);

    gtk_accel_group_connect(accel, GDK_KEY_j, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_j);
    gtk_accel_group_connect(accel, GDK_KEY_k, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_k);

    gtk_window_add_accel_group(GTK_WINDOW(self), accel);

    g_closure_unref(closure_j);
    g_closure_unref(closure_k);
}

static void srain_window_class_init(SrainWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, about_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, connect_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, spinner);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, stack);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_icon);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, quit_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, about_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, sidebar_box);
}

SrainWindow* srain_window_new(SrainApp *app){
    return g_object_new(SRAIN_TYPE_WINDOW, "application", app, NULL);
}

void srain_window_add_buffer(SrainWindow *win, SrainBuffer *buffer){
    GString *gstr;

    gstr = g_string_new("");
    g_string_printf(gstr, "%s %s",
            srain_buffer_get_remark(buffer),
            srain_buffer_get_name(buffer));
    gtk_stack_add_named(win->stack, GTK_WIDGET(buffer), gstr->str);
    g_string_free(gstr, TRUE);

    theme_apply(GTK_WIDGET(win));

    gtk_stack_set_visible_child(win->stack, GTK_WIDGET(buffer));
}

void srain_window_rm_buffer(SrainWindow *win, SrainBuffer *buffer){
    // srain_user_list_clear(srain_buffer_get_user_list(buffer));
    gtk_container_remove(GTK_CONTAINER(win->stack), GTK_WIDGET(buffer));
}

SrainBuffer* srain_window_get_cur_buffer(SrainWindow *win){
    SrainBuffer *buffer;

    buffer = SRAIN_BUFFER(gtk_stack_get_visible_child(win->stack));

    return buffer;
}

SrainServerBuffer* srain_window_get_cur_server_buffer(SrainWindow *win){
    SrainBuffer *buffer;

    buffer = srain_window_get_cur_buffer(win);
    if (!SRAIN_IS_BUFFER(buffer)){
        return NULL;
    }
    if (SRAIN_IS_SERVER_BUFFER(buffer)){
        return SRAIN_SERVER_BUFFER(buffer);
    }
    if (SRAIN_IS_CHAT_BUFFER(buffer)){
        return srain_chat_buffer_get_server_buffer(SRAIN_CHAT_BUFFER(buffer));
    }

    return NULL;
}

SrainBuffer* srain_window_get_buffer(SrainWindow *win,
        const char *name, const char *remark){
    SrainBuffer *buffer;

    GString *fullname = g_string_new("");
    g_string_printf(fullname, "%s %s", remark, name);
    buffer = SRAIN_BUFFER(gtk_stack_get_child_by_name(win->stack, fullname->str));
    g_string_free(fullname, TRUE);

    return buffer;
}

void srain_window_spinner_toggle(SrainWindow *win, gboolean is_busy){
   is_busy
        ? gtk_spinner_start(win->spinner)
        : gtk_spinner_stop(win->spinner);
}

void srain_window_stack_sidebar_update(SrainWindow *win, SrainBuffer *buffer,
        const char *nick, const char *msg){
    if (SRAIN_BUFFER(gtk_stack_get_visible_child(win->stack)) != buffer){
        srain_stack_sidebar_update(win->sidebar, buffer, nick, msg, 0);
    } else {
        srain_stack_sidebar_update(win->sidebar, buffer, nick, msg, 1);
    }
}

void srain_window_tray_icon_stress(SrainWindow *win, int stress){
    gtk_status_icon_set_from_icon_name(win->tray_icon, stress ? "srain-red": "srain");
}

int srain_window_is_active(SrainWindow *win){
    int active;

    g_object_get(G_OBJECT(win), "is-active", &active, NULL);

    return active;
}

SrainConnectPopover *srain_window_get_connect_popover(SrainWindow *win){
    return win->connect_popover;
}

SrainJoinPopover *srain_window_get_join_popover(SrainWindow *win){
    return win->join_popover;
}
