/**
 * @file ui_test.c
 * @brief
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-28
 */

#define __DBG_ON
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

int ui_test_srv_cmd(const char *srv_name, const char *chan_name, char *cmd, int block){
    return 0;
}


void ui_test(){
    // TODO: add assert
    assert(ui_add_chan_sync("irc.freenode.net", "#srain", "la", CHAT_CHANNEL) == 0);
    assert(ui_add_chan_sync("irc.freenode.net", "#la", "la", CHAT_CHANNEL) == 0);
    assert(ui_add_chan_sync("irc.freenode.net", "#srain", "la", CHAT_CHANNEL) == -1);
    assert(ui_rm_chan_sync("irc.freenode.net", "#sraiN") == -1);
    assert(ui_rm_chan_sync("irc.freenode.net", "#srain") == 0);
    assert(ui_rm_chan_sync("irc.freenode.net", "#la") == 0);

    assert(ui_add_chan_sync("irc.freenode.net", "#srain", "la", CHAT_CHANNEL) == 0);
    assert(ui_add_chan_sync("chat.freenode.net", "#srain", "la", CHAT_CHANNEL) == 0);
    assert(ui_add_chan_sync("irc.freenode.net", "#srain2", "la", CHAT_CHANNEL) == 0);

    // irc.freenode.net #srain
    // chat.freenode.net #srain
    // irc.freenode.net #srain2

    assert(ui_add_chan_sync("irc.freenode.net", "#summer-time-record", "la", CHAT_CHANNEL) == 0);
    ui_set_topic_sync("irc.freenode.net", "#summer-time-record",
            "Summer Time Record （夏令时记录）");

    ui_sys_msg_sync("no.exist.net", "#summer-time-record", "follback to cur chan", SYS_MSG_NORMAL);
    ui_send_msg_sync("no.exist.net", "#summer-time-record", "msg");
    ui_recv_msg_sync("no.exist.net", "#summer-time-record", "nick", "id", "msg");

    ui_sys_msg_sync("irc.freenode.net", "#summer-time-record",
            "マリー：痛いくらいに现実は 足早に駆け抜けた", SYS_MSG_NORMAL);
    ui_send_msg_sync("irc.freenode.net", "#summer-time-record",
            "マリー ナイス！");
    ui_recv_msg_sync("irc.freenode.net", "#summer-time-record",
            "マリー", "", "えへへっ");
    ui_sys_msg_sync("irc.freenode.net", "#summer-time-record",
            "カノ：选んだ今日は平凡で 崩れそうになる日々さ", SYS_MSG_ACTION);
    ui_send_msg_sync("irc.freenode.net", "#summer-time-record",
            "なんかイラッとするっすね");
    ui_recv_msg_sync("irc.freenode.net", "#summer-time-record",
            "キド", "", "奇遇だな?俺もそう思った");
    ui_recv_msg_sync("irc.freenode.net", "#summer-time-record",
            "カノ", "", "ヒドくないっ！");
    ui_sys_msg_sync("irc.freenode.net", "#summer-time-record",
            "ヒビヤ：昨日の今日も延长戦 大人だって 臆病だ", SYS_MSG_ERROR);
    ui_recv_msg_sync("irc.freenode.net", "#summer-time-record",
            "モモ", "", "ヒビヤでうだうめい");
    ui_recv_msg_sync("irc.freenode.net", "#summer-time-record",
            "ヒビヤ", "", "バスッ!");

    // Test ui_user_list_add
    assert(ui_user_list_add_sync("chat.freenode.net", "#non-exist",
                "la", USER_FULL_OP) == -1);

    // Test srain_user_list_add
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "la", USER_FULL_OP) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "la2", USER_CHIGUA) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "la", USER_FULL_OP) == -1);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "la", USER_CHIGUA) == -1);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "la4", USER_CHIGUA) == 0);

    // Test ui_user_list_rm: la, la2, la4
    assert(ui_user_list_rm_sync("chat.freenode.net", "#non-exist",
                "la") == -1);

    // Test srain_user_list_rm: la, la2, la4
    assert(ui_user_list_rm_sync("chat.freenode.net", "#srain",
                "la3") == -1);
    assert(ui_user_list_rm_sync("chat.freenode.net", "#srain",
                "la") == 0);

    // Test ui_user_list_rename: la2, la4
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la2", "la", USER_FULL_OP);
    // Test srain_user_list_rename: la2, la4
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la2", "la", USER_FULL_OP);
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la4", "la", USER_FULL_OP);
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la5", "la6", USER_FULL_OP);
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la", "la2", USER_CHIGUA);
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la2", "la2", USER_CHIGUA);
    ui_user_list_rename_sync("chat.freenode.net", "#srain",
                "la2", "la2", USER_FULL_OP);

    // Clear list
    assert(ui_user_list_rm_sync("chat.freenode.net", "#srain",
                "la2") == 0);
    assert(ui_user_list_rm_sync("chat.freenode.net", "#srain",
                "la4") == 0);

    // Test SrainUserList sort function: <empty>
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "chigua", USER_CHIGUA) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "owner", USER_OWNER) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "voiced2", USER_VOICED) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "owner2", USER_OWNER) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "admin2", USER_ADMIN) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "voiced", USER_VOICED) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "chigua2", USER_CHIGUA) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "full-op2", USER_FULL_OP) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "half-op", USER_HALF_OP) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "half-op2", USER_HALF_OP) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "admin", USER_ADMIN) == 0);
    assert(ui_user_list_add_sync("chat.freenode.net", "#srain",
                "full-op", USER_FULL_OP) == 0);
}

