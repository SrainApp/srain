#define __LOG_ON

#include <assert.h>

#include "ui.h"
#include "log.h"
#include "irc_magic.h"

void ui_test_server_join(const char *server_name, const char *chan_name){
    LOG_FR("server_name: %s, chan_name: %s", server_name, chan_name);
}
void ui_test_server_part(const char *server_name, const char *chan_name){
    LOG_FR("server_name: %s, chan_name: %s", server_name, chan_name);
}
void ui_test_server_send(const char *server_name, const char *chan_name, const char *msg){
    LOG_FR("server_name: %s, chan_name: %s, msg: '%s'", server_name, chan_name, msg);
}
void ui_test_server_cmd(const char *server_name, const char *chan_name, const char *cmd){
    LOG_FR("server_name: %s, chan_name: %s, cmd: '%s'", server_name, chan_name, cmd);
}

void ui_test(){
    assert(ui_add_chan("irc.freenode.net", "#srain") == 0);
    assert(ui_add_chan("irc.freenode.net", "#la") == 0);
    assert(ui_add_chan("irc.freenode.net", "#srain") == -1);
    assert(ui_rm_chan("irc.freenode.net", "#sraiN") == -1);
    assert(ui_rm_chan("irc.freenode.net", "#srain") == 0);
    assert(ui_rm_chan("irc.freenode.net", "#la") == 0);

    assert(ui_add_chan("irc.freenode.net", "#srain") == 0);
    assert(ui_add_chan("chat.freenode.net", "#srain") == 0);
    assert(ui_add_chan("irc.freenode.net", "#srain2") == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain", "la", USER_OP) == 0);
    LOG_FR("%d", ui_user_list_add("chat.freenode.net", "#srain", "la", USER_OP) == -1);
}

