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

    // irc.freenode.net #srain
    // chat.freenode.net #srain
    // irc.freenode.net #srain2

    assert(ui_add_chan("irc.freenode.net", "#summer-time-record") == 0);
    ui_sys_msg("no.exist.net", "#summer-time-record", "follback to cur chan", SYS_MSG_NORMAL);
    ui_send_msg("no.exist.net", "#summer-time-record", "msg");
    ui_recv_msg("no.exist.net", "#summer-time-record", "nick", "id", "msg");

    ui_sys_msg("irc.freenode.net", "#summer-time-record",
            "マリー：痛いくらいに现実は 足早に駆け抜けた", SYS_MSG_NORMAL);
    ui_send_msg("irc.freenode.net", "#summer-time-record",
            "マリー ナイス！");
    ui_recv_msg("irc.freenode.net", "#summer-time-record",
            "マリー", "", "えへへっ");
    ui_sys_msg("irc.freenode.net", "#summer-time-record",
            "カノ：选んだ今日は平凡で 崩れそうになる日々さ", SYS_MSG_ACTION);
    ui_send_msg("irc.freenode.net", "#summer-time-record",
            "なんかイラッとするっすね");
    ui_recv_msg("irc.freenode.net", "#summer-time-record",
            "キド", "", "奇遇だな?俺もそう思った");
    ui_recv_msg("irc.freenode.net", "#summer-time-record",
            "カノ", "", "ヒドくないっ！");
    ui_sys_msg("irc.freenode.net", "#summer-time-record",
            "ヒビヤ：昨日の今日も延长戦 大人だって 臆病だ", SYS_MSG_ERROR);
    ui_recv_msg("irc.freenode.net", "#summer-time-record",
            "モモ", "", "ヒビヤでうだうめい");
    ui_recv_msg("irc.freenode.net", "#summer-time-record",
            "ヒビヤ", "", "バスッ!");

    // Test ui_user_list_add
    assert(ui_user_list_add("chat.freenode.net", "#non-exist",
                            "la", USER_FULL_OP) == -1);

    // Test srain_user_list_add
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_FULL_OP) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la2", USER_CHIGUA) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_FULL_OP) == -1);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_CHIGUA) == -1);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "la4", USER_CHIGUA) == 0);

    // Test ui_user_list_rm: la, la2, la4
    assert(ui_user_list_rm("chat.freenode.net", "#non-exist",
                           "la", "REASON") == -1);

    // Test srain_user_list_rm: la, la2, la4
    assert(ui_user_list_rm("chat.freenode.net", "#srain",
                           "la3", "REASON") == -1);
    assert(ui_user_list_rm("chat.freenode.net", "#srain",
                           "la", "REASON") == 0);

    // Test ui_user_list_rename: la2, la4
    assert(ui_user_list_rename("chat.freenode.net", "#non-exist",
                               "la2", "la", USER_FULL_OP) == -1);

    // Test srain_user_list_rename: la2, la4
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                               "la2", "la", USER_FULL_OP) == 0);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                               "la4", "la", USER_FULL_OP) == -1);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                               "la5", "la6", USER_FULL_OP) == -1);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                               "la", "la2", USER_CHIGUA) == 0);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                               "la2", "la2", USER_CHIGUA) == 0);
    assert(ui_user_list_rename("chat.freenode.net", "#srain",
                               "la2", "la2", USER_FULL_OP) == 0);

    // Clear list
    assert(ui_user_list_rm("chat.freenode.net", "#srain",
                           "la2", "REASON") == 0);
    assert(ui_user_list_rm("chat.freenode.net", "#srain",
                           "la4", "REASON") == 0);

    // Test SrainUserList sort function: <empty>
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "chigua", USER_CHIGUA) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "owner", USER_OWNER) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "voiced2", USER_VOICED) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "owner2", USER_OWNER) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "admin2", USER_ADMIN) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "voiced", USER_VOICED) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "chigua2", USER_CHIGUA) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "full-op2", USER_FULL_OP) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "half-op", USER_HALF_OP) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "half-op2", USER_HALF_OP) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "admin", USER_ADMIN) == 0);
    assert(ui_user_list_add("chat.freenode.net", "#srain",
                            "full-op", USER_FULL_OP) == 0);
}

