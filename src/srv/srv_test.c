#define __LOG_ON

#include <assert.h>

#include "srv.h"
#include "srv_test.h"
#include "srv_session.h"

#include "log.h"
#include "meta.h"

void srv_test_ui_add_chan(const char *srv_name, const char *chan_name){
    LOG_FR("srv_name: %s, chan_name: %s", srv_name, chan_name);
}

void srv_test_ui_rm_chan(const char *srv_name, const char *chan_name){
    LOG_FR("srv_name: %s, chan_name: %s", srv_name, chan_name);
}

void srv_test_ui_sys_msg(const char *srv_name, const char *chan_name,
        const char *msg, SysMsgType type){
    LOG_FR("srv_name: %s, chan_name: %s, msg: '%s', type: %d",
            srv_name, chan_name, msg, type);
}

void srv_test_ui_send_msg(const char *srv_name, const char *chan_name,
        const char *msg){
    LOG_FR("srv_name: %s, chan_name: %s, msg: '%s'",
            srv_name, chan_name, msg);
}

void srv_test_ui_recv_msg(const char *srv_name, const char *chan_name,
        const char *nick, const char *id, const char *msg){
    LOG_FR("srv_name: %s, chan_name: %s, nick: %s, id: %s, msg: '%s'",
            srv_name, chan_name, nick, id, msg);
}

void srv_test_ui_user_list_add(const char *srv_name, const char *chan_name,
        const char *nick, UserType type){
    LOG_FR("srv_name: %s, chan_name: %s, nick: '%s', type: %d",
            srv_name, chan_name, nick, type);
}

void srv_test_ui_user_list_rm(const char *srv_name, const char *chan_name,
        const char *nick){
    LOG_FR("srv_name: %s, chan_name: %s, nick: '%s'",
            srv_name, chan_name, nick);
}

void srv_test_ui_user_list_rename(const char *srv_name, const char *chan_name, 
        const char *old_nick, const char *new_nick, UserType type){
    LOG_FR("srv_name: %s, chan_name: %s, old_nick: %s, new_nick: %s, type: %d",
            srv_name, chan_name, old_nick, new_nick, type);
}

void srv_test_ui_set_topic(const char *srv_name, const char *chan_name,
        const char *topic){
    LOG_FR("srv_name: %s, chan_name: %s, topic: '%s'",
            srv_name, chan_name, topic);
}

void srv_test(){
    assert(srv_connect("127.0.0.1", 6667, NULL, "la", PACKAGE_VERSION, PACKAGE_WEBSITE, SSL_OFF) == 0);
    assert(srv_connect("localhost", 6667, NULL, "srainbot2", PACKAGE_VERSION, PACKAGE_WEBSITE, SSL_OFF) == 0);

    while (1);
}
