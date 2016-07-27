#define __LOG_ON

#include <assert.h>

#include "ui.h"
#include "log.h"

int ui_test_srv_join(const char *srv_name, const char *chan_name,
        const char *passwd){
    return 0;
}

int ui_test_srv_part(const char *srv_name, const char *chan_name,
        const char *reason){
    return 0;
}

int ui_test_srv_send(const char *srv_name, const char *chan_name, const char *msg){
    return 0;
}

int ui_test_srv_cmd(const char *srv_name, const char *chan_name, const char *cmd){
    return 0;
}


void ui_test(){
    // TODO: add assert
    ui_add_chan("irc.freenode.net", "#srain");
    ui_add_chan("irc.freenode.net", "#la");
    ui_add_chan("irc.freenode.net", "#srain");
    ui_rm_chan("irc.freenode.net", "#sraiN");
    ui_rm_chan("irc.freenode.net", "#srain");
    ui_rm_chan("irc.freenode.net", "#la");

    ui_add_chan("irc.freenode.net", "#srain");
    ui_add_chan("chat.freenode.net", "#srain");
    ui_add_chan("irc.freenode.net", "#srain2");

    // irc.freenode.net #srain
    // chat.freenode.net #srain
    // irc.freenode.net #srain2

    ui_add_chan("irc.freenode.net", "#summer-time-record");
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
    ui_user_list_add("chat.freenode.net", "#non-exist",
                            "la", USER_FULL_OP);

    // Test srain_user_list_add
    ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_FULL_OP);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "la2", USER_CHIGUA);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_FULL_OP);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "la", USER_CHIGUA);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "la4", USER_CHIGUA);

    // Test ui_user_list_rm: la, la2, la4
    ui_user_list_rm("chat.freenode.net", "#non-exist", "la");

    // Test srain_user_list_rm: la, la2, la4
    ui_user_list_rm("chat.freenode.net", "#srain", "la3");
    ui_user_list_rm("chat.freenode.net", "#srain", "la");

    // Test ui_user_list_rename: la2, la4
    ui_user_list_rename("chat.freenode.net", "la2", "la", USER_FULL_OP,
            "la2 -> la");

    // Test srain_user_list_rename: la2, la4
    ui_user_list_rename("chat.freenode.net", "la2", "la", USER_FULL_OP,
            "la2 -> la");
    ui_user_list_rename("chat.freenode.net", "la4", "la", USER_FULL_OP,
            "la4 -> la");
    ui_user_list_rename("chat.freenode.net", "la5", "la6", USER_FULL_OP,
            "la5 -> la6");
    ui_user_list_rename("chat.freenode.net", "la", "la2", USER_CHIGUA,
            "la -> la2");
    ui_user_list_rename("chat.freenode.net", "la2", "la2", USER_CHIGUA,
            "la2 -> la2");
    ui_user_list_rename("chat.freenode.net", "la2", "la2", USER_FULL_OP,
            "la2 -> la2");

    // Clear list
    ui_user_list_rm("chat.freenode.net", "#srain", "la2");
    ui_user_list_rm("chat.freenode.net", "#srain", "la4");

    // Test SrainUserList sort function: <empty>
    ui_user_list_add("chat.freenode.net", "#srain",
                            "chigua", USER_CHIGUA);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "owner", USER_OWNER);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "voiced2", USER_VOICED);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "owner2", USER_OWNER);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "admin2", USER_ADMIN);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "voiced", USER_VOICED);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "chigua2", USER_CHIGUA);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "full-op2", USER_FULL_OP);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "half-op", USER_HALF_OP);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "half-op2", USER_HALF_OP);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "admin", USER_ADMIN);
    ui_user_list_add("chat.freenode.net", "#srain",
                            "full-op", USER_FULL_OP);
}

