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
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
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

static void srain_app_activate(GtkApplication *app){

    if (srain_win){
        gtk_window_present(GTK_WINDOW(srain_win));
    } else {
        srain_win = srain_window_new(SRAIN_APP(app));
        gtk_window_present(GTK_WINDOW(srain_win));

        sui_event_hdr(NULL, SUI_EVENT_ACTIVATE, NULL, 0);
    }
}

static void srain_app_init(SrainApp *self){
    if (srain_app) return;

    srain_app = self;
    return;
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate =
        (void *)(GApplication *)srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP,
            "application-id", "org.gtk.srain", NULL);
}

void srain_app_quit(SrainApp *app){
    GtkWidget *win;
    GList *list, *next;

    list = gtk_application_get_windows(GTK_APPLICATION(app));
    while (list){
        win = list->data;
        next = list->next;

        gtk_widget_destroy (GTK_WIDGET (win));

        list = next;
    }
}
