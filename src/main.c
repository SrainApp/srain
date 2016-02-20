#include <stdio.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "i18n.h"
#include "ui.h"
#include "srain_app.h"
#include "srain_window.h"
#include "log.h"
#include "irc.h"
#include "srain.h"
#include "config.h"

int main(int argc, char **argv){
    return g_application_run(G_APPLICATION(srain_app_new()), argc, argv);
}
