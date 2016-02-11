#include <stdio.h>
#include <gtk/gtk.h>
#include "ui.h"
#include "log.h"

/*
 * 
*/
MsgRecv msg1 = {
    .id = "Emily",
    .nick = "Bindiger",
    .avatar = NULL,
    .img = "./img.png",
    .msg = "in my long forgotten cloistered sleep,\n\
you and I were resting close in peace\n\
was it just a dreaming of my heart?",
    .chan = "#lasttest",
    .time = "02-01 20:08"
};

MsgSend msg2 = {
    .time = "02-01 20:08",
    .nick = "Bindiger",
    .img = "./img.png",
    .chan = "#lasttest",
    .msg = "now I'm crying' don't know why\n\
where do all the tears come from?\n\
could no one ever dry up the spring?"
};

MsgSys msg3 = {
    .msg = "if you find me crying in the dark\n\
please call my name' from the heart",
    .chan = "#lasttest"
};

int main(int argc, char **argv){
    gtk_init(&argc, &argv);
    ui_window_init();
    ui_msg_init();

    ui_chat_add("#lasttest", "LastAvengers' test channel");
    ui_msg_recv(&msg1);
    msg1.img = NULL;
    ui_msg_send(msg2);
    msg2.img = NULL;
    ui_msg_send(msg2);
    ui_msg_sys(&msg3);
    ui_msg_recv(&msg1);
    gtk_main();

    return 0;
}
