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
#include "sui_event_hdr.h"
#include "theme.h"
#include "sui_window.h"
#include "sui_buffer.h"
#include "sui_connect_panel.h"
#include "srain_join_popover.h"
#include "sui_side_bar.h"
#include "tray_icon.h"

#define SEND_MESSAGE_INTERVAL       100

#define WINDOW_STACK_PAGE_WELCOME   "welcome"
#define WINDOW_STACK_PAGE_MAIN      "main"

struct _SuiWindow {
    GtkApplicationWindow parent;

    SuiWindowEvents *events;
    SuiWindowConfig *cfg;

    /* Tray icon, TODO */
    GtkStatusIcon *tray_icon;
    GtkMenu *tray_menu;
    GtkMenuItem *about_menu_item;
    GtkMenuItem *quit_menu_item;

    /* Top level container */
    GtkPaned *title_paned;
    GtkBox *window_box;
    GtkSeparator *header_separator;
    GtkStack *header_box;
    GtkStack *header_paned;
    GtkStack *window_stack;

    /* Side header */
    GtkHeaderBar *side_header_bar;
    GtkBox *side_header_box;
    GtkBox *side_left_header_box;
    GtkBox *side_right_header_box;
    GtkButton *start_button;
    GtkButton *connect_button;
    GtkButton *join_button;

    /* Buffer header */
    GtkHeaderBar *buffer_header_bar;
    GtkBox *buffer_header_box;
    GtkBox *buffer_title_box;
    GtkLabel *buffer_title_label;
    GtkLabel *buffer_subtitle_label;
    GtkMenuButton *buffer_menu_button;

    /* Welcome page */
    GtkBox *welcome_connect_box;

    /* Main page */
    GtkPaned *main_paned;
    GtkBox *side_box;
    SuiSideBar *side_bar;
    GtkStack *buffer_stack;
    GtkButton *plugin_button;
    GtkTextView *input_text_view;
    GtkButton *send_button;
    int send_timer;

    /* Popovers */
    SuiConnectPanel *connect_panel;
    SrainJoinPopover *join_popover;
};

struct _SuiWindowClass {
    GtkApplicationWindowClass parent_class;
};

static void sui_window_set_events(SuiWindow *self, SuiWindowEvents *events);

static void update_header(SuiWindow *self);
static void update_title(SuiWindow *self);
static void show_about_dialog(gpointer user_data);
static int get_buffer_count(SuiWindow *self);
static void send_message_cancel(SuiWindow *self);
static void send_message(SuiWindow *self);

static void on_destroy(SuiWindow *self);
static void on_notify_is_active(GObject *object, GParamSpec *pspec,
        gpointer data);

static void window_stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data);
static void buffer_stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data);
static void quit_menu_item_on_activate();
static void popover_button_on_click(GtkButton *button, gpointer user_data);
static gboolean CTRL_J_K_on_press(GtkAccelGroup *group, GObject *obj,
        guint keyval, GdkModifierType mod, gpointer user_data);
static gboolean input_text_view_on_key_press(GtkTextView *text_view,
        GdkEventKey *event, gpointer user_data);
static void send_button_on_clicked(GtkWidget *widget, gpointer user_data);
static gboolean send_message_timeout(gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_EVENTS = 1,
  PROP_CONFIG,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(SuiWindow, sui_window, GTK_TYPE_APPLICATION_WINDOW);

static void sui_window_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiWindow *self;

  self = SUI_WINDOW(object);

  switch (property_id){
    case PROP_EVENTS:
      sui_window_set_events(self, g_value_get_pointer(value));
      break;
    case PROP_CONFIG:
      sui_window_set_config(self, g_value_get_pointer(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_window_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiWindow *self;

  self = SUI_WINDOW(object);

  switch (property_id){
    case PROP_EVENTS:
      g_value_set_pointer(value, sui_window_get_events(self));
      break;
    case PROP_CONFIG:
      g_value_set_pointer(value, sui_window_get_config(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_window_init(SuiWindow *self){
    GClosure *closure_j;
    GClosure *closure_k;
    GtkAccelGroup *accel;

    gtk_widget_init_template(GTK_WIDGET(self));

    /* Bind title_paned, header_paned and main_paned */
    g_object_bind_property(
            self->title_paned, "position",
            self->main_paned, "position",
            G_BINDING_BIDIRECTIONAL);
    g_object_bind_property(
            self->header_paned, "position",
            self->main_paned, "position",
            G_BINDING_BIDIRECTIONAL);

    self->send_timer = 0;

    /* Stack side bar init */
    self->side_bar = sui_side_bar_new();
    gtk_box_pack_start(self->side_box, GTK_WIDGET(self->side_bar),
            TRUE, TRUE, 0);
    sui_side_bar_set_stack(self->side_bar, self->buffer_stack);
    gtk_widget_show(GTK_WIDGET(self->side_bar));

    /* Popover init */
    self->connect_panel = g_object_ref(sui_connect_panel_new());
    self->join_popover = srain_join_popover_new();

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
    g_signal_connect_swapped(self->start_button, "clicked",
            G_CALLBACK(show_about_dialog), self);
    g_signal_connect(self->connect_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->connect_panel);
    g_signal_connect(self->join_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->join_popover);

    g_signal_connect(self->window_stack, "notify::visible-child",
            G_CALLBACK(window_stack_on_child_changed), self);
    g_signal_connect(self->buffer_stack, "notify::visible-child",
            G_CALLBACK(buffer_stack_on_child_changed), self);

    g_signal_connect(self->input_text_view, "key-press-event",
            G_CALLBACK(input_text_view_on_key_press), self);
    g_signal_connect(self->send_button, "clicked",
            G_CALLBACK(send_button_on_clicked), self);

    /* shortcut <C-j> and <C-k> */
    accel = gtk_accel_group_new();

    closure_j = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press),
            self->side_bar, NULL);
    closure_k = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press),
            self->side_bar, NULL);

    gtk_accel_group_connect(accel, GDK_KEY_j, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_j);
    gtk_accel_group_connect(accel, GDK_KEY_k, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_k);

    gtk_window_add_accel_group(GTK_WINDOW(self), accel);

    g_closure_unref(closure_j);
    g_closure_unref(closure_k);

}

static void sui_window_constructed(GObject *object){
    SuiWindow *self;

    self = SUI_WINDOW(object);
    if (!self->cfg->csd){
        /* Move side header widgets from side_header_bar to side_header_box */
        gtk_container_remove(GTK_CONTAINER(self->side_header_bar),
                GTK_WIDGET(self->side_left_header_box));
        gtk_container_remove(GTK_CONTAINER(self->side_header_bar),
                GTK_WIDGET(self->side_right_header_box));
        gtk_box_pack_start(self->side_header_box,
                GTK_WIDGET(self->side_left_header_box), TRUE, TRUE, 0);
        gtk_box_pack_end(self->side_header_box,
                GTK_WIDGET(self->side_right_header_box), TRUE, TRUE, 0);
        gtk_container_child_set(GTK_CONTAINER(self->side_header_box),
                GTK_WIDGET(self->side_right_header_box), "expand", FALSE, NULL);

        /* Move buffer header widgets from buffer_header_bar to buffer_header_box */
        gtk_header_bar_set_custom_title(self->buffer_header_bar, NULL);
        gtk_container_remove(GTK_CONTAINER(self->buffer_header_bar),
                GTK_WIDGET(self->buffer_menu_button));
        gtk_box_set_center_widget(self->buffer_header_box,
                GTK_WIDGET(self->buffer_title_box));
        gtk_box_pack_end(self->buffer_header_box,
                GTK_WIDGET(self->buffer_menu_button), TRUE, TRUE, 0);
        gtk_container_child_set(GTK_CONTAINER(self->buffer_header_box),
                GTK_WIDGET(self->buffer_menu_button), "expand", FALSE, NULL);

        // Hide the titlebar node
        gtk_window_set_titlebar(GTK_WINDOW(self), NULL);
        // Show the seperator
        gtk_widget_show(GTK_WIDGET(self->header_separator));
    }
    update_header(self);
    update_title(self);

    G_OBJECT_CLASS(sui_window_parent_class)->constructed(object);
}

static void sui_window_class_init(SuiWindowClass *class){
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = sui_window_constructed;
    object_class->set_property = sui_window_set_property;
    object_class->get_property = sui_window_get_property;

    /* Install properties */
    obj_properties[PROP_EVENTS] =
        g_param_spec_pointer("events",
                "Events",
                "Event callbacks of window.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_CONFIG] =
        g_param_spec_pointer("config",
                "Config",
                "Configuration of window.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class, N_PROPERTIES,
            obj_properties);

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class, "/im/srain/Srain/window.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiWindow, tray_icon);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, tray_menu);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, about_menu_item);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, quit_menu_item);

    gtk_widget_class_bind_template_child(widget_class, SuiWindow, title_paned);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, window_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, header_separator);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, header_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, header_paned);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, window_stack);

    gtk_widget_class_bind_template_child(widget_class, SuiWindow, side_header_bar);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, side_header_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, side_left_header_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, side_right_header_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, start_button);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, connect_button);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, join_button);

    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_header_bar);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_header_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_title_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_title_label);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_subtitle_label);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_menu_button);

    gtk_widget_class_bind_template_child(widget_class, SuiWindow, welcome_connect_box);

    gtk_widget_class_bind_template_child(widget_class, SuiWindow, main_paned);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, side_box);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, buffer_stack);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, plugin_button);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, input_text_view);
    gtk_widget_class_bind_template_child(widget_class, SuiWindow, send_button);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiWindow* sui_window_new(SuiApplication *app, SuiWindowEvents *events,
        SuiWindowConfig *cfg){
    SuiWindow *self;

    cfg->csd = TRUE;
    self = g_object_new(SUI_TYPE_WINDOW,
            "application", app,
            "events", events,
            "config", cfg,
            NULL);

    return self;
}

void sui_window_add_buffer(SuiWindow *self, SuiBuffer *buf){
    GString *gstr;

    if (get_buffer_count(self) == 0){
        gtk_stack_set_visible_child_name(
                self->window_stack, WINDOW_STACK_PAGE_MAIN);
    }

    gstr = g_string_new("");
    g_string_printf(gstr, "%s %s",
            sui_buffer_get_remark(buf),
            sui_buffer_get_name(buf));
    gtk_stack_add_named(self->buffer_stack, GTK_WIDGET(buf), gstr->str);
    g_string_free(gstr, TRUE);

    theme_apply(GTK_WIDGET(self));

    gtk_stack_set_visible_child(self->buffer_stack, GTK_WIDGET(buf));
}

void sui_window_rm_buffer(SuiWindow *self, SuiBuffer *buf){
    // srain_user_list_clear(sui_buffer_get_user_list(buf));
    gtk_container_remove(GTK_CONTAINER(self->buffer_stack), GTK_WIDGET(buf));

    if (get_buffer_count(self) == 0){
        gtk_stack_set_visible_child_name(
                self->window_stack, WINDOW_STACK_PAGE_WELCOME);
    }
}

SuiBuffer* sui_window_get_cur_buffer(SuiWindow *self){
    SuiBuffer *buf;

    buf = SUI_BUFFER(gtk_stack_get_visible_child(self->buffer_stack));

    return buf;
}

SuiBuffer* sui_window_get_buffer(SuiWindow *self,
        const char *name, const char *remark){
    SuiBuffer *buf;

    GString *fullname = g_string_new("");
    g_string_printf(fullname, "%s %s", remark, name);
    buf = SUI_BUFFER(gtk_stack_get_child_by_name(self->buffer_stack, fullname->str));
    g_string_free(fullname, TRUE);

    return buf;
}

SuiSideBar* sui_window_get_side_bar(SuiWindow *self){
    return self->side_bar;
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

SuiWindowEvents* sui_window_get_events(SuiWindow *self) {
    return self->events;
}

void sui_window_set_config(SuiWindow *self, SuiWindowConfig *cfg) {
    self->cfg = cfg;
}

SuiWindowConfig* sui_window_get_config(SuiWindow *self) {
    return self->cfg;
}

void sui_window_set_title(SuiWindow *self, const char *title){
    gtk_label_set_text(self->buffer_title_label, title);
}

void sui_window_set_subtitle(SuiWindow *self, const char *subtitle){
    gtk_label_set_text(self->buffer_subtitle_label, subtitle);
}


SuiConnectPanel *sui_window_get_connect_panel(SuiWindow *self){
    return self->connect_panel;
}

SrainJoinPopover *sui_window_get_join_popover(SuiWindow *self){
    return self->join_popover;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_window_set_events(SuiWindow *self, SuiWindowEvents *events) {
    self->events = events;
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

static void update_header(SuiWindow *self){
    const char *page;

    page = gtk_stack_get_visible_child_name(self->window_stack);
    if (g_strcmp0(page, WINDOW_STACK_PAGE_WELCOME) == 0){
        gtk_widget_set_visible(GTK_WIDGET(self->connect_button), FALSE);
        gtk_widget_set_visible(GTK_WIDGET(self->join_button), FALSE);
        if (self->cfg->csd){
            gtk_widget_set_visible(GTK_WIDGET(self->buffer_header_bar), FALSE);
            gtk_header_bar_set_show_close_button(self->side_header_bar, TRUE);
        } else {
            gtk_widget_set_visible(GTK_WIDGET(self->buffer_header_box), FALSE);
        }

        // Add connect panel to welcome page
        gtk_box_pack_start(self->welcome_connect_box,
                GTK_WIDGET(self->connect_panel), TRUE, TRUE, 0);
    } else if (g_strcmp0(page, WINDOW_STACK_PAGE_MAIN) == 0){
        gtk_widget_set_visible(GTK_WIDGET(self->connect_button), TRUE);
        gtk_widget_set_visible(GTK_WIDGET(self->join_button), TRUE);
        if (self->cfg->csd){
            gtk_header_bar_set_show_close_button(self->side_header_bar, FALSE);
            gtk_widget_set_visible(GTK_WIDGET(self->buffer_header_bar), TRUE);
        } else {
            gtk_widget_set_visible(GTK_WIDGET(self->buffer_header_box), TRUE);
        }

        // Remove connect panel to welcome page
        gtk_container_remove(GTK_CONTAINER(self->welcome_connect_box),
                GTK_WIDGET(self->connect_panel));
    } else {
        g_warn_if_reached();
    }
}

static void update_title(SuiWindow *self){
    const char *page;

    page = gtk_stack_get_visible_child_name(self->window_stack);
    if (g_strcmp0(page, WINDOW_STACK_PAGE_WELCOME) == 0){
        gtk_window_set_title(GTK_WINDOW(self), PACKAGE_NAME);
        if (self->cfg->csd){
            gtk_header_bar_set_title(self->side_header_bar, PACKAGE_NAME);
        }
    } else if (g_strcmp0(page, WINDOW_STACK_PAGE_MAIN) == 0){
        char *title;

        title = g_strdup_printf("%s @ %s - %s",
                gtk_label_get_text(self->buffer_title_label),
                gtk_label_get_text(self->buffer_subtitle_label),
                PACKAGE_NAME);
        gtk_window_set_title(GTK_WINDOW(self), title);
        if (self->cfg->csd){
            gtk_header_bar_set_title(self->side_header_bar, NULL);
            gtk_header_bar_set_title(self->buffer_header_bar, title);
        }
        g_free(title);
    } else {
        g_warn_if_reached();
    }
}

static int get_buffer_count(SuiWindow *self){
    return g_list_length(
            gtk_container_get_children(GTK_CONTAINER(self->buffer_stack)));
}

static void send_message(SuiWindow *self){
    g_return_if_fail(!self->send_timer);

    gtk_text_view_set_editable(self->input_text_view, FALSE); // Lock text view
    self->send_timer = g_timeout_add(
            SEND_MESSAGE_INTERVAL, send_message_timeout, self);

    gtk_image_set_from_icon_name(
            GTK_IMAGE(gtk_button_get_image(self->send_button)),
            "dialog-cancel", GTK_ICON_SIZE_BUTTON);
}

static void send_message_cancel(SuiWindow *self){
    g_return_if_fail(self->send_timer);

    g_source_remove(self->send_timer);
    self->send_timer = 0;
    gtk_text_view_set_editable(self->input_text_view, TRUE); // Unlock text view

    gtk_image_set_from_icon_name(
            GTK_IMAGE(gtk_button_get_image(self->send_button)),
            "dialog-ok", GTK_ICON_SIZE_BUTTON);
}

static void on_destroy(SuiWindow *self){
    // Nothing to do for now
}

static void on_notify_is_active(GObject *object, GParamSpec *pspec,
        gpointer data){
   if (sui_window_is_active(SUI_WINDOW(object))){
       /* Stop stress the icon */
       sui_window_tray_icon_stress(SUI_WINDOW(object), 0);
   } else {

   }
}

static void popover_button_on_click(GtkButton *button, gpointer user_data){
    GtkPopover *popover;

    popover = GTK_POPOVER(user_data);
    sui_panel_popup(GTK_WIDGET(button), GTK_WIDGET(popover));
}

static void quit_menu_item_on_activate(){
    // sui_application_quit(srain_app);
    // FIXME
}

static gboolean CTRL_J_K_on_press(GtkAccelGroup *group, GObject *obj,
        guint keyval, GdkModifierType mod, gpointer user_data){
    SuiSideBar *side_bar;

    if (mod != GDK_CONTROL_MASK) return FALSE;

    side_bar = user_data;
    switch (keyval){
        case GDK_KEY_k:
            sui_side_bar_prev(side_bar);
            break;
        case GDK_KEY_j:
            sui_side_bar_next(side_bar);
            break;
        default:
            ERR_FR("unknown keyval %d", keyval);
            return FALSE;
    }

    return TRUE;
}

static gboolean input_text_view_on_key_press(GtkTextView *text_view,
        GdkEventKey *event, gpointer user_data){
    SuiWindow *self;

    self = SUI_WINDOW(user_data);
    if (event->keyval != GDK_KEY_Return){
        return FALSE;
    }
    if ((self->cfg->send_on_ctrl_enter) ^ (event->state & GDK_CONTROL_MASK )){
        // TODO: filter SHIFT, ALT and META?
        return FALSE;
    }

    gtk_button_clicked(self->send_button);

    return TRUE;
}

static void send_button_on_clicked(GtkWidget *widget, gpointer user_data){
    SuiWindow *self;

    self = SUI_WINDOW(user_data);

    if (self->send_timer == 0) {
        send_message(self);
    } else {
        send_message_cancel(self);
    }
}

static gboolean send_message_timeout(gpointer user_data){
    int nline;
    char *line;
    GVariantDict *params;
    GtkTextIter start;
    GtkTextIter end;
    GtkTextBuffer *text_buffer;
    SrnRet ret;
    SuiWindow *self;
    SuiBuffer *buf;

    self = SUI_WINDOW(user_data);
    buf = sui_window_get_cur_buffer(self);
    if (!SUI_IS_BUFFER(buf)) {
        goto FIN;
    }
    text_buffer = gtk_text_view_get_buffer(self->input_text_view);

    nline = gtk_text_buffer_get_line_count(text_buffer);
    gtk_text_buffer_get_iter_at_line(text_buffer, &start, 0);
    gtk_text_buffer_get_iter_at_line(text_buffer, &end, 1);
    line = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);
    if (strlen(line) == 0 && nline == 1){ // Text buffer is empty
        g_free(line);
        goto FIN;
    }

    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "message", SUI_EVENT_PARAM_STRING, line);
    ret = sui_buffer_event_hdr(buf, SUI_EVENT_SEND, params);
    g_variant_dict_unref(params);
    g_free(line);

    if (!RET_IS_OK(ret)){
        goto FIN;
    }

    // Delete the sent line
    gtk_text_buffer_delete(text_buffer, &start, &end);

    return G_SOURCE_CONTINUE;

FIN:
    send_message_cancel(self);
    return G_SOURCE_REMOVE;
}

static void window_stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data){
    SuiWindow *self;

    self = user_data;
    update_header(self);
    update_title(self);
}

static void buffer_stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data){
    SuiWindow *self;
    SuiBuffer *buf;

    self = user_data;
    buf = sui_window_get_cur_buffer(self);
    if (!SUI_IS_BUFFER(buf)){
        return;
    }

    sui_window_set_title(self, sui_buffer_get_name(buf));
    sui_window_set_subtitle(self, sui_buffer_get_remark(buf));
    update_title(self);
    gtk_text_view_set_buffer(self->input_text_view,
            sui_buffer_get_input_text_buffer(buf));
    gtk_menu_button_set_popup(self->buffer_menu_button,
            GTK_WIDGET(sui_buffer_get_menu(buf)));
}
