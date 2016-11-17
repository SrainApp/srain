#define __LOG_ON

#include <assert.h>

#include "srv.h"
#include "srv_test.h"
#include "srv_session.h"

#include "log.h"
#include "meta.h"

void srv_test_ui_add_chat(SIGN_UI_ADD_CHAT){
    LOG_FR("srv_name: %s, chat_name: %s, nick: %s, type: %d",
            srv_name, chat_name, nick, type);
}

void srv_test_ui_rm_chat(SIGN_UI_RM_CHAT){
    LOG_FR("srv_name: %s, chat_name: %s", srv_name, chat_name);
}

void srv_test_ui_sys_msg(SIGN_UI_SYS_MSG){
    LOG_FR("srv_name: %s, chat_name: %s, msg: '%s', type: %d",
            srv_name, chat_name, msg, type);
}

void srv_test_ui_send_msg(SIGN_UI_SEND_MSG){
    LOG_FR("srv_name: %s, chat_name: %s, msg: '%s'",
            srv_name, chat_name, msg);
}

void srv_test_ui_recv_msg(SIGN_UI_RECV_MSG){
    LOG_FR("srv_name: %s, chat_name: %s, nick: %s, id: %s, msg: '%s'",
            srv_name, chat_name, nick, id, msg);
}

void srv_test_ui_add_user(SIGN_UI_ADD_USER){
    LOG_FR("srv_name: %s, chat_name: %s, nick: '%s', type: %d",
            srv_name, chat_name, nick, type);
}

void srv_test_ui_rm_user(SIGN_UI_RM_USER){
    LOG_FR("srv_name: %s, chat_name: %s, nick: '%s'",
            srv_name, chat_name, nick);
}

void srv_test_ui_ren_user(SIGN_UI_REN_USER){
    LOG_FR("srv_name: %s, chat_name: %s, old_nick: %s, new_nick: %s, type: %d",
            srv_name, chat_name, old_nick, new_nick, type);
}

void srv_test_ui_set_topic(SIGN_UI_SET_TOPIC){
    LOG_FR("srv_name: %s, chat_name: %s, topic: '%s'",
            srv_name, chat_name, topic);
}

void srv_test(){
    assert(srv_connect("127.0.0.1", 6667, NULL, "la", PACKAGE_VERSION, PACKAGE_WEBSITE, 0) == 0);
    assert(srv_connect("localhost", 6667, NULL, "srainbot2", PACKAGE_VERSION, PACKAGE_WEBSITE, 0) == 0);

    while (1);
}
