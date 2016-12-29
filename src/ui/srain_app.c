/**
 * @file srain_app.c
 * @brief Srain's application class implement
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>

#include "theme.h"
#include "ui_test.h"
#include "ui_common.h"
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

#ifdef UI_TEST
        ui_test();
#endif

        if (rc_read() < 0) {
            /* Show a message dialog if no command runned in rc file */
            show_msg_dialog(_("Welcome"), _("<big><b>Welcome to Srain :)</b></big>\n\n"
                "Click the \"Connect\" button on the header bar to connect to a IRC server."
                "\n\n"
                "If you want to execute some commands automatically when startup, please "
                "add your commands into <u>~/.config/srain/srainrc</u>."
                "\n\n"
                "Need help? Please visit "
                "<a href=\"https://github.com/silverrainz/srain/wiki\">Srain's website</a>."
                "\n"));
        }
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
