#define __LOG_ON

#include <assert.h>

#include "ui.h"
#include "log.h"
#include "irc_magic.h"

void ui_test_server_join(const char *server_name, const char *chan_name){
    LOG_FR("server_name: %s, chan_name: %s", server_name, chan_name);
    ui_add_chan(server_name, chan_name);
}

void ui_test_server_part(const char *server_name, const char *chan_name){
    LOG_FR("server_name: %s, chan_name: %s", server_name, chan_name);
    ui_rm_chan("server_name", "chan_name");
}

void ui_test_server_send(const char *server_name, const char *chan_name, const char *msg){
    LOG_FR("server_name: %s, chan_name: %s, msg: '%s'", server_name, chan_name, msg);
    ui_send_msg(server_name, chan_name, msg);
}

void ui_test_server_cmd(const char *server_name, const char *chan_name, const char *cmd){
    LOG_FR("server_name: %s, chan_name: %s, cmd: '%s'", server_name, chan_name, cmd);
}

void ui_test(){
    return;
    assert(ui_add_chan("irc.freenode.net", "#srain") == 0);
    assert(ui_add_chan("irc.freenode.net", "#la") == 0);
    assert(ui_add_chan("irc.freenode.net", "#srain") == -1);
    assert(ui_rm_chan("irc.freenode.net", "#sraiN") == -1);
    assert(ui_rm_chan("irc.freenode.net", "#srain") == 0);
    assert(ui_rm_chan("irc.freenode.net", "#la") == 0);

    assert(ui_add_chan("irc.freenode.net", "#srain") == 0);
    assert(ui_add_chan("chat.freenode.net", "#srain") == 0);
    assert(ui_add_chan("irc.freenode.net", "#srain2") == 0);

    // irc.freenode.net #srain
    // chat.freenode.net #srain
    // irc.freenode.net #srain2

    // Test ui_user_list_add
    assert(ui_user_list_add("chat.freenode.net", "#non-exist",
                            "la", USER_OP) == -1);

    // Test srain_user_list_add
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_OP) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la2", USER_PERSON) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_OP) == -1);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_PERSON) == -1);

    // Test ui_user_list_rm
    assert(ui_user_list_rm("chat.freenode.net", "#non-exist",
                           "la", "REASON") == -1);

    // Test srain_user_list_rm
    assert(ui_user_list_rm("chat.freenode.net", "#srain",
                           "la3", "REASON") == -1);
    assert(ui_user_list_rm("chat.freenode.net", "#srain",
                           "la", "REASON") == 0);

    // Test ui_user_list_rename
    assert(ui_user_list_rename("chat.freenode.net", "#non-exist",
                            "la2", "la", USER_OP) == -1);

    // Test srain_user_list_rename
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                            "la2", "la", USER_OP) == 0);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                            "la", "la2", USER_PERSON) == 0);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                            "la3", "la", USER_OP) == -1);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                            "la3", "la", USER_PERSON) == -1);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                            "la2", "la2", USER_PERSON) == -1);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                            "la2", "la2", USER_OP) == -1);
}

