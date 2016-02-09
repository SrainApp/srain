#include <stdio.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "i18n.h"
#include "ui.h"
#include "log.h"
#include "irc.h"
#include "srain.h"

int main(int argc, char **argv){
    i18n_init();
    gtk_init(&argc, &argv);
    printf(_("Srain!\n"));

    ui_window_init();
    srain_login("srainbot");
    ui_msg_init();
    srain_join("#lasttest");
    srain_listen();
    gtk_main();

    return 0;
}
