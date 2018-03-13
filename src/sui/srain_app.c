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
#include "theme.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "srain_app.h"
#include "srain_window.h"

#include "meta.h"
#include "log.h"
#include "i18n.h"

struct _SrainApp {
    GtkApplication parent;
    SuiApplication *ctx;
};

struct _SrainAppClass {
    GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

/* Only one SrainApp instance in one application */
static SrainApp *app_instance = NULL;

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

static SrnRet create_window(GApplication *app);
static void on_activate(GApplication *app);
static void on_shutdown(GApplication *app);
static int on_handle_local_options(GApplication *app, GVariantDict *options,
        gpointer user_data);
static int on_command_line(GApplication *app,
        GApplicationCommandLine *cmdline, gpointer user_data);

static void srain_app_init(SrainApp *self){
    g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

    g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(self, "shutdown", G_CALLBACK(on_shutdown), NULL);
    g_signal_connect(self, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(self, "handle-local-options", G_CALLBACK(on_handle_local_options), NULL);

    return;
}

static void srain_app_class_init(SrainAppClass *class){
}

SrainApp* srain_app_new(SuiApplication *ctx, const char *id){
    if (app_instance == NULL) {
        app_instance = g_object_new(SRAIN_TYPE_APP,
                "application-id", id,
                "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                NULL);
    }
    app_instance->ctx = ctx;

    return app_instance;
}

SrainApp* srain_app_get_default(){
    return app_instance;
}

void srain_app_quit(SrainApp *app){
    /*
    GtkWidget *win;
    GList *list, *next;

    list = gtk_application_get_windows(GTK_APPLICATION(app));
    while (list){
        win = list->data;
        next = list->next;
        gtk_widget_destroy (GTK_WIDGET (win));
        list = next;
    }
    */
    g_application_quit(G_APPLICATION(app));
}

SuiApplication* srain_app_get_ctx(SrainApp *app){
    return app->ctx;
}

SrnApplication* srain_app_get_core_ctx(SrainApp *app){
    return (SrnApplication *)sui_application_get_ctx(app->ctx);
}

static SrnRet create_window(GApplication *app){
    // FIXME: config
    // SrainWindow *win;
    // win = srain_window_new(SRAIN_APP(app));
    // gtk_window_present(GTK_WINDOW(win));

    return SRN_OK;
}

static void on_activate(GApplication *app){
    SrnRet ret;

    ret = create_window(app);
    if (!RET_IS_OK(ret)){
        return;
    }

    ret = sui_application_event_hdr(SRAIN_APP(app)->ctx, SUI_EVENT_ACTIVATE, NULL);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void on_shutdown(GApplication *app){
    sui_application_event_hdr(SRAIN_APP(app)->ctx, SUI_EVENT_SHUTDOWN, NULL);
}

static int on_handle_local_options(GApplication *app, GVariantDict *options,
        gpointer user_data){
    if (g_variant_dict_lookup(options, "version", "b", NULL)){
        g_print("%s %s%s\n", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUILD);
        return 0; // Exit
    }

    return -1; // Return -1 to let the default option processing continue.
}

static int on_command_line(GApplication *app,
        GApplicationCommandLine *cmdline, gpointer user_data){
    char **urls;
    GVariantDict *options;
    GVariantDict* params;

    options = g_application_command_line_get_options_dict(cmdline);
    if (g_variant_dict_lookup(options, G_OPTION_REMAINING, "^as", &urls)){
        /* If we have URLs to open, create window firstly. */
        create_window(app);

        params = g_variant_dict_new(NULL);
        g_variant_dict_insert(params, "urls", SUI_EVENT_PARAM_STRINGS,
                urls, g_strv_length(urls));

        sui_application_event_hdr(SRAIN_APP(app)->ctx, SUI_EVENT_OPEN, params);

        g_variant_dict_unref(params);
        g_strfreev(urls);
    }

    g_application_activate(app);

    return 0;
}
