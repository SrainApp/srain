#include <stdio.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "i18n.h"
#include "ui.h"
#include "log.h"
#include "irc.h"
#include "srain.h"

int main(int argc, char **argv){
    IRC irc;

    i18n_init();
    // irc_connect(&irc, "irc.freenode.net", "6666");
    // irc_login("srainbot");
    printf(_("Srain!\n"));

    gtk_init(&argc, &argv);
    ui_window_init();
    ui_msg_init();

    srain_init();
    srain_join("#lasttest");
    srain_listen();
    gtk_main();

    return 0;
}
