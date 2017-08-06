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
 * @file srain_app.c
 * @brief Srain's application class implementation
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06
 * @date 2016-03-01
 */

#include <gtk/gtk.h>

#include "theme.h"
#include "sui_common.h"
#include "sui_event_hdr.h"
#include "srain_app.h"
#include "srain_window.h"

#include "meta.h"
#include "rc.h"
#include "log.h"
#include "i18n.h"

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

/* Only one SrainApp instance in one application */
SrainApp *srain_app = NULL;
/* Only one SrainWindow instance in one application */
SrainWindow *srain_win = NULL;

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
        .arg_description = N_("[URL...]")
    },
    {NULL}
};

static SrnRet create_window(GApplication *app){
    if (srain_win){
        gtk_window_present(GTK_WINDOW(srain_win));
        return SRN_ERR;
    };

    srain_win = srain_window_new(SRAIN_APP(app));
    gtk_window_present(GTK_WINDOW(srain_win));

    return SRN_OK;
}

static void activate(GApplication *app){
    if (RET_IS_OK(create_window(app))){
        sui_event_hdr(NULL, SUI_EVENT_ACTIVATE, NULL, 0);
    }
}

static gint handle_local_options(GApplication *app, GVariantDict *options,
        gpointer user_data){
    if (g_variant_dict_lookup(options, "version", "b", NULL)){
        g_print("%s %s%s\n", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUILD);
        return 0; // Exit
    }

    return -1; // Return -1 to let the default option processing continue.
}

static gint command_line(GApplication *app,
        GApplicationCommandLine *cmdline, gpointer user_data){
    int len;
    gchar **urls;
    GVariantDict *options;

    options = g_application_command_line_get_options_dict(cmdline);

    if (g_variant_dict_lookup(options, G_OPTION_REMAINING, "^as", &urls)){
        /* If we have URLs to open, activate app firstly. */
        create_window(app);

        len =  g_strv_length(urls);
        sui_event_hdr(NULL, SUI_EVENT_OPEN, (const char **)urls, len);
        g_strfreev(urls);
    }

    g_application_activate(app);

    return 0;
}

static void srain_app_init(SrainApp *self){
    if (srain_app) return;

    srain_app = self;

    g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

    g_signal_connect(self, "activate", G_CALLBACK(activate), NULL);
    g_signal_connect(self, "command-line", G_CALLBACK(command_line), NULL);
    g_signal_connect(self, "handle-local-options", G_CALLBACK(handle_local_options), NULL);

    return;
}

static void srain_app_class_init(SrainAppClass *class){
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP,
            "application-id", "org.srain.srain",
            "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
            NULL);
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
