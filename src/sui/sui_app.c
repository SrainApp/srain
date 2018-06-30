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
 * @file sui_app.c
 * @brief Sui module application class implementation
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "meta.h"
#include "log.h"
#include "i18n.h"

#include "sui_theme.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_app.h"
#include "sui_window.h"

struct _SuiApplication {
    GtkApplication parent;

    SuiApplicationEvents *events;
    SuiApplicationConfig *cfg;
    SuiThemeManager *theme;
    void *ctx;
};

struct _SuiApplicationClass {
    GtkApplicationClass parent_class;
};

/* Only one SuiApplication instance in one application */
static SuiApplication *app_instance = NULL;

static GOptionEntry option_entries[] = {
    {
        .long_name = "version",
        .short_name = 'v',
        .flags = 0,
        .arg = G_OPTION_ARG_NONE,
        .arg_data = NULL,
        .description = N_("Show version information"),
        .arg_description = NULL,
    },
    {
        .long_name = G_OPTION_REMAINING,
        .short_name = '\0',
        .flags = 0,
        .arg = G_OPTION_ARG_STRING_ARRAY,
        .arg_data = NULL,
        .description = N_("Open one or more IRC URLs"),
        .arg_description = N_("[URL…]")
    },
    {NULL}
};

static void sui_application_set_ctx(SuiApplication *self, void *ctx);
static void sui_application_set_events(SuiApplication *self,
        SuiApplicationEvents *events);

static void on_startup(SuiApplication *self);
static void on_activate(SuiApplication *self);
static void on_shutdown(SuiApplication *self);
static int on_handle_local_options(SuiApplication *self, GVariantDict *options,
        gpointer user_data);
static int on_command_line(SuiApplication *self,
        GApplicationCommandLine *cmdline, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_CTX = 1,
  PROP_EVENTS,
  PROP_CONFIG,
  N_PROPERTIES
};

G_DEFINE_TYPE(SuiApplication, sui_application, GTK_TYPE_APPLICATION);

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void sui_application_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiApplication *self = SUI_APPLICATION(object);

  switch (property_id){
    case PROP_CTX:
      sui_application_set_ctx(self, g_value_get_pointer(value));
      break;
    case PROP_EVENTS:
      sui_application_set_events(self, g_value_get_pointer(value));
      break;
    case PROP_CONFIG:
      sui_application_set_config(self, g_value_get_pointer(value));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_application_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiApplication *self = SUI_APPLICATION(object);

  switch (property_id){
    case PROP_CTX:
      g_value_set_pointer(value, sui_application_get_ctx(self));
      break;
    case PROP_EVENTS:
      g_value_set_pointer(value, sui_application_get_events(self));
      break;
    case PROP_CONFIG:
      g_value_set_pointer(value, sui_application_get_config(self));
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
}

static void sui_application_init(SuiApplication *self){
    self->theme = sui_theme_manager_new();

    g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

    g_signal_connect(self, "startup", G_CALLBACK(on_startup), NULL);
    g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(self, "shutdown", G_CALLBACK(on_shutdown), NULL);
    g_signal_connect(self, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(self, "handle-local-options",
            G_CALLBACK(on_handle_local_options), NULL);
}

static void sui_application_constructed(GObject *object){
    G_OBJECT_CLASS(sui_application_parent_class)->constructed(object);
}

static void sui_application_finalize(GObject *object){
    SuiApplication *self;

    self = SUI_APPLICATION(object);

    sui_theme_manager_free(self->theme);

    G_OBJECT_CLASS(sui_application_parent_class)->finalize(object);
}

static void sui_application_class_init(SuiApplicationClass *class){
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = sui_application_constructed;
    object_class->finalize = sui_application_finalize;
    object_class->set_property = sui_application_set_property;
    object_class->get_property = sui_application_get_property;

    /* Install properties */
    obj_properties[PROP_CTX] =
        g_param_spec_pointer("context",
                "Context",
                "Context of application.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_EVENTS] =
        g_param_spec_pointer("events",
                "Events",
                "Event callbacks of application.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

    obj_properties[PROP_CONFIG] =
        g_param_spec_pointer("config",
                "Config",
                "Configuration of application.",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiApplication* sui_application_new(const char *id, void *ctx,
        SuiApplicationEvents *events, SuiApplicationConfig *cfg){
    if (app_instance == NULL) {
        app_instance = g_object_new(SUI_TYPE_APPLICATION,
                "application-id", id,
                "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                "context", ctx,
                "events", events,
                "config", cfg,
                NULL);
    }

    return app_instance;
}

void sui_application_run(SuiApplication *self, int argc, char *argv[]){
    g_return_if_fail(SUI_IS_APPLICATION(self));

    g_application_run(G_APPLICATION(self), argc, argv);
}

void sui_application_quit(SuiApplication *self){
    g_return_if_fail(SUI_IS_APPLICATION(self));
    /*
    GtkWidget *win;
    GList *list, *next;

    list = gtk_application_get_windows(GTK_APPLICATION(self));
    while (list){
        win = list->data;
        next = list->next;
        gtk_widget_destroy (GTK_WIDGET (win));
        list = next;
    }
    */
    g_application_quit(G_APPLICATION(self));
}

/**
 * @brief ``sui_application_send_notification``
 *
 * @param self
 * @param msg
 */
void sui_application_send_notification(SuiApplication *self,
        SuiNotification *notif){
    GIcon *icon;
    GNotification *gnotif;

    g_return_if_fail(SUI_IS_APPLICATION(self));
    g_return_if_fail(notif);

    icon = g_themed_icon_new(notif->icon);
    g_return_if_fail(icon);

    gnotif = g_notification_new(notif->title);
    g_notification_set_body(gnotif, notif->body);
    g_notification_set_icon(gnotif, icon);

    sui_window_tray_icon_stress(sui_get_cur_window(), 1); // FIXME
    g_application_send_notification(G_APPLICATION(self), notif->id, gnotif);

    g_object_unref(gnotif);
    g_object_unref(icon);
}

SuiApplication* sui_application_get_instance(){
    return app_instance;
}

SuiWindow* sui_application_get_cur_window(SuiApplication *self){
    return SUI_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(self)));
}

void* sui_application_get_ctx(SuiApplication *self){
    return self->ctx;
}

SuiApplicationEvents* sui_application_get_events(SuiApplication *self){
    return self->events;
}

void sui_application_set_config(SuiApplication *self, SuiApplicationConfig *cfg){
    self->cfg = cfg;
}

SuiApplicationConfig* sui_application_get_config(SuiApplication *self){
    return self->cfg;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void sui_application_set_ctx(SuiApplication *self, void *ctx){
    self->ctx = ctx;
}

static void sui_application_set_events(SuiApplication *self,
        SuiApplicationEvents *events){
    self->events = events;
}

static void on_startup(SuiApplication *self){
    SrnRet ret;

    ret = sui_theme_manager_apply(self->theme, self->cfg->theme);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void on_activate(SuiApplication *self){
    SrnRet ret;

    ret = sui_application_event_hdr(self, SUI_EVENT_ACTIVATE, NULL);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void on_shutdown(SuiApplication *self){
    sui_application_event_hdr(self, SUI_EVENT_SHUTDOWN, NULL);
}

static int on_handle_local_options(SuiApplication *self, GVariantDict *options,
        gpointer user_data){
    if (g_variant_dict_lookup(options, "version", "b", NULL)){
        g_print("%s %s%s\n", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUILD);
        return 0; // Exit
    }

    return -1; // Return -1 to let the default option processing continue.
}

static int on_command_line(SuiApplication *self,
        GApplicationCommandLine *cmdline, gpointer user_data){
    char **urls;
    GVariantDict *options;
    GVariantDict* params;
    SrnRet ret;

    // Activate application firstly, it will create window if not exist
    g_application_activate(G_APPLICATION(self));

    options = g_application_command_line_get_options_dict(cmdline);
    if (g_variant_dict_lookup(options, G_OPTION_REMAINING, "^as", &urls)){
        params = g_variant_dict_new(NULL);
        g_variant_dict_insert(params, "urls", SUI_EVENT_PARAM_STRINGS,
                urls, g_strv_length(urls));

        ret = sui_application_event_hdr(self, SUI_EVENT_OPEN, params);
        if (!RET_IS_OK(ret)){
            sui_message_box(_("Error"), RET_MSG(ret));
        }

        g_variant_dict_unref(params);
        g_strfreev(urls);
    }

    return 0;
}
