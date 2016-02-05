#include <stdio.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "i18n.h"
#include "ui.h"
#include "log.h"
#include "irc.h"

int main(int argc, char **argv){
    IRC irc;
    char in[128];

    i18n_init();
    // irc_connect(&irc, "irc.freenode.net", "6666");
    // irc_login("srainbot");
    printf(_("Srain!\n"));

    gtk_init(&argc, &argv);
    ui_window_init();
    ui_msg_init();

    gtk_main();

    return 0;
}
