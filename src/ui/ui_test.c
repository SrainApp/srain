#include "log.h"
#include "ui.h"

void ui_test_server_name_join(const char *server_name, const char *chan_name){
    LOG_FR("server_name: %s, chan_name: %s", server_name, chan_name);
}
void ui_test_server_name_part(const char *server_name, const char *chan_name){
    LOG_FR("server_name: %s, chan_name: %s", server_name, chan_name);
}
void ui_test_server_name_send(const char *server_name, const char *chan_name, const char *msg){
    LOG_FR("server_name: %s, chan_name: %s, msg: '%s'", server_name, chan_name, msg);
}
void ui_test_server_name_cmd(const char *server_name, const char *chan_name, const char *cmd){
    LOG_FR("server_name: %s, chan_name: %s, cmd: '%s'", server_name, chan_name, cmd);
}

void ui_test(){
    ui_add_chan("irc.freenode.net", "#srain");
}

