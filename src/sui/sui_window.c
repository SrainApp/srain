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
 * @file sui_window.c
 * @brief Sui window class
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include <string.h>
#include <assert.h>

#include "core/core.h"
#include "sui/sui.h"
#include "meta.h"
#include "log.h"
#include "i18n.h"

#include "sui_common.h"
#include "theme.h"
#include "sui_window.h"
#include "srain_buffer.h"
#include "srain_server_buffer.h"
#include "srain_chat_buffer.h"
#include "srain_connect_popover.h"
#include "srain_join_popover.h"
#include "srain_stack_sidebar.h"
#include "tray_icon.h"

struct _SuiWindow {
    GtkApplicationWindow parent;

    SuiWindowEvents *events;
    SuiWindowConfig *cfg;
    void *ctx;

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

struct _SuiWindowClass {
    GtkApplicationWindowClass parent_class;
};

static void quit_menu_item_on_activate();
static void on_destroy(SuiWindow *self);
static void on_notify_is_active(GObject *object, GParamSpec *pspec,
        gpointer data);
static void show_about_dialog(gpointer user_data);
static void popover_button_on_click(gpointer user_data);
static gboolean CTRL_J_K_on_press(GtkAccelGroup *group, GObject *obj,
        guint keyval, GdkModifierType mod, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiWindow, sui_window, GTK_TYPE_APPLICATION_WINDOW);

static void sui_window_init(SuiWindow *self){
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

    g_signal_connect(self, "destroy",
            G_CALLBACK(on_destroy), NULL);
    g_signal_connect(self, "notify::is-active",
            G_CALLBACK(on_notify_is_active), NULL);

    g_signal_connect_swapped(self->quit_menu_item, "activate",
            G_CALLBACK(quit_menu_item_on_activate), NULL);
    g_signal_connect_swapped(self->about_menu_item, "activate",
            G_CALLBACK(show_about_dialog), self);

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

static void sui_window_class_init(SuiWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, about_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, join_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, connect_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, spinner);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, stack);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, tray_icon);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, tray_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, quit_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, about_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SuiWindow, sidebar_box);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiWindow* sui_window_new(SuiApplication *app, SuiWindowEvents *events,
        SuiWindowConfig *cfg){
    SuiWindow *self;

    self = g_object_new(SUI_TYPE_WINDOW, "application", app, NULL);
    self->events = events;
    self->cfg = cfg;

    return self;
}
SuiWindowEvents* sui_window_get_events(SuiWindow *sui) {
    return sui->events;
}

void* sui_window_get_ctx(SuiWindow *self){
    return self->ctx;
}

void sui_window_set_ctx(SuiWindow *self, void *ctx){
    self->ctx = ctx;
}

void sui_window_add_buffer(SuiWindow *self, SuiBuffer *buf){
    GString *gstr;

    gstr = g_string_new("");
    g_string_printf(gstr, "%s %s",
            sui_buffer_get_remark(buf),
            sui_buffer_get_name(buf));
    gtk_stack_add_named(self->stack, GTK_WIDGET(buf), gstr->str);
    g_string_free(gstr, TRUE);

    theme_apply(GTK_WIDGET(self));

    gtk_stack_set_visible_child(self->stack, GTK_WIDGET(buf));
}

void sui_window_rm_buffer(SuiWindow *self, SuiBuffer *buf){
    // srain_user_list_clear(sui_buffer_get_user_list(buf));
    gtk_container_remove(GTK_CONTAINER(self->stack), GTK_WIDGET(buf));
}

SuiBuffer* sui_window_get_cur_buffer(SuiWindow *self){
    SuiBuffer *buf;

    buf = SUI_BUFFER(gtk_stack_get_visible_child(self->stack));

    return buf;
}

SrainServerBuffer* sui_window_get_cur_server_buffer(SuiWindow *self){
    SuiBuffer *buf;

    buf = sui_window_get_cur_buffer(self);
    if (!SUI_IS_BUFFER(buf)){
        return NULL;
    }
    if (SRAIN_IS_SERVER_BUFFER(buf)){
        return SRAIN_SERVER_BUFFER(buf);
    }
    if (SRAIN_IS_CHAT_BUFFER(buf)){
        return srain_chat_buffer_get_server_buffer(SRAIN_CHAT_BUFFER(buf));
    }

    return NULL;
}

SuiBuffer* sui_window_get_buffer(SuiWindow *self,
        const char *name, const char *remark){
    SuiBuffer *buf;

    GString *fullname = g_string_new("");
    g_string_printf(fullname, "%s %s", remark, name);
    buf = SUI_BUFFER(gtk_stack_get_child_by_name(self->stack, fullname->str));
    g_string_free(fullname, TRUE);

    return buf;
}

void sui_window_spinner_toggle(SuiWindow *self, gboolean is_busy){
   is_busy
        ? gtk_spinner_start(self->spinner)
        : gtk_spinner_stop(self->spinner);
}

void sui_window_stack_sidebar_update(SuiWindow *self, SuiBuffer *buf,
        const char *nick, const char *msg){
    if (SUI_BUFFER(gtk_stack_get_visible_child(self->stack)) != buf){
        srain_stack_sidebar_update(self->sidebar, buf, nick, msg, 0);
    } else {
        srain_stack_sidebar_update(self->sidebar, buf, nick, msg, 1);
    }
}

void sui_window_tray_icon_stress(SuiWindow *self, int stress){
    gtk_status_icon_set_from_icon_name(self->tray_icon,
            stress ? "srain-red": "im.srain.Srain");
}

int sui_window_is_active(SuiWindow *self){
    int active;

    g_object_get(G_OBJECT(self), "is-active", &active, NULL);

    return active;
}

SrainConnectPopover *sui_window_get_connect_popover(SuiWindow *self){
    return self->connect_popover;
}

SrainJoinPopover *sui_window_get_join_popover(SuiWindow *self){
    return self->join_popover;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void quit_menu_item_on_activate(){
    // sui_application_quit(srain_app);
    // FIXME
}

static void on_destroy(SuiWindow *self){
    // Nothing to do for now
}

static void on_notify_is_active(GObject *object, GParamSpec *pspec,
        gpointer data ) {
   if (sui_window_is_active(SUI_WINDOW(object))){
       /* Stop stress the icon */
       sui_window_tray_icon_stress(SUI_WINDOW(object), 0);
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
            "logo-icon-name", "im.srain.Srain",
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
