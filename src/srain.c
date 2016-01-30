#include <stdio.h>
#include <gtk/gtk.h>
#include "i18n.h"
#include "ui.h"


int main(int argc, char **argv){
    const char* chat_list[] = {
        "#archlinux-cn",
        "#opensuse-cn",
        "#kernel",
        "#gzlug",
        NULL
    };

    i18n_init();
    printf(_("Hello srain!\n"));

    gtk_init(&argc, &argv);
    ui_window_init();

    int i;
    for(i = 0; chat_list[i] != NULL; i++ ){
        ui_join_chan(chat_list[i]);
    }

    gtk_main();

    return 0;
}
