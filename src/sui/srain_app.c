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
 * @file srain_app.c
 * @brief Srain's application class implementation
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "meta.h"
#include "log.h"
#include "i18n.h"

#include "theme.h"
#include "snotify.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "srain_app.h"
#include "srain_window.h"


struct _SuiApplication {
    GtkApplication parent;

    SuiApplicationEvents *events;
    SuiApplicationConfig *cfg;
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

static void on_activate(SuiApplication *self);
static void on_shutdown(SuiApplication *self);
static int on_handle_local_options(SuiApplication *self, GVariantDict *options,
        gpointer user_data);
static int on_command_line(SuiApplication *self,
        GApplicationCommandLine *cmdline, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiApplication, sui_application, GTK_TYPE_APPLICATION);

static void sui_application_init(SuiApplication *self){
    g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

    g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(self, "shutdown", G_CALLBACK(on_shutdown), NULL);
    g_signal_connect(self, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(self, "handle-local-options", G_CALLBACK(on_handle_local_options), NULL);
}

static void sui_application_class_init(SuiApplicationClass *class){}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiApplication* sui_application_new(const char *id,
        SuiApplicationEvents *events, SuiApplicationConfig *cfg){
    if (app_instance == NULL) {
        app_instance = g_object_new(SUI_TYPE_APPLICATION,
                "application-id", id,
                "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                NULL);
        app_instance->events = events;
        app_instance->cfg = cfg;
    }

    return app_instance;
}

void sui_application_run(SuiApplication *self, int argc, char *argv[]){
    snotify_init();

    if (theme_load(self->cfg->theme) == SRN_ERR){
        char *errmsg;

        errmsg = g_strdup_printf(_("Failed to load theme \"%1$s\""),
                self->cfg->theme);
        sui_message_box(_("Error"), errmsg);
        g_free(errmsg);
    }

    g_application_run(G_APPLICATION(self), argc, argv);
    snotify_finalize();
}

void sui_application_quit(SuiApplication *self){
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

SuiApplication* sui_application_get_instance(){
    return app_instance;
}

SuiWindow* sui_application_get_cur_window(SuiApplication *self){
    return SUI_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(self)));
}

SuiApplicationEvents* sui_application_get_events(SuiApplication *self){
    return self->events;
}

void* sui_application_get_ctx(SuiApplication *self){
    return self->ctx;
}

void sui_application_set_ctx(SuiApplication *self, void *ctx){
    self->ctx = ctx;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

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
