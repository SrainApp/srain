/**
 * @file ui_hdr.c
 * @brief IRC server interface for UI module
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-21
 */

// #define __DBG_ON
#define __LOG_ON

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "srain_app.h"
#include "srain_window.h"
#include "srain_chat.h"
#include "ui_hdr.h"
#include "ui_test.h"

#include "srv.h"

#include "log.h"

/* import from srain_app.c */
extern SrainApp *srain_app;
extern SrainWindow *srain_win;

/* Raw interface of srv module */
SRVJoinFunc _ui_hdr_srv_join;
SRVPartFunc _ui_hdr_srv_part;
SRVSendFunc _ui_hdr_srv_send;
SRVCmdFunc  _ui_hdr_srv_cmd;

void ui_hdr_init(){
#ifdef UI_TEST
    _ui_hdr_srv_join = ui_test_srv_join;
    _ui_hdr_srv_part = ui_test_srv_part;
    _ui_hdr_srv_send = ui_test_srv_send;
    _ui_hdr_srv_cmd =  ui_test_srv_cmd;
#else
    _ui_hdr_srv_join = srv_join;
    _ui_hdr_srv_part = srv_part;
    _ui_hdr_srv_send = srv_send;
    _ui_hdr_srv_cmd =  srv_cmd;
#endif
}

/**
 * @brief Server Interface to execute a command
 *
 * @param source where this cmd comes from,
 *      if NULL, command echo may appear in the current SrainChat
 * @param cmd the command you want to execute
 *
 * @return 0 if successed, -1 if failed
 */
int ui_hdr_srv_cmd(SrainChat *chat, char *cmd, int block){
    int res;
    char *cmd2;
    const char *srv_name;
    const char *chat_name;


    if (chat == NULL){
        chat = srain_window_get_cur_chat(srain_win);
    }

    if (chat){
        srv_name = srain_chat_get_srv_name(chat);
        chat_name = srain_chat_get_name(chat);
    } else {
        srv_name = chat_name = NULL;
    }

    DBG_FR("srv_name: %s, chat_name: %s, cmd: '%s', block: %d",
            srv_name, chat_name, cmd, block);

    cmd2 = strdup(cmd);
    res = _ui_hdr_srv_cmd(srv_name, chat_name, cmd2, block);
    free(cmd2);

    return res;
}

/**
 * @brief join a chatnel of current server
 *
 * @param chat_name
 */
int ui_hdr_srv_join(const char *chat_name, const char *passwd){
    const char *srv_name;
    SrainChat *chat;

    chat = srain_window_get_cur_chat(srain_win);
    g_return_val_if_fail(chat, -1);
    srv_name = srain_chat_get_srv_name(chat);

    DBG_FR("srv_name: %s, chat_name: %s, passwd: %s",
            srv_name, chat_name, passwd);

    return _ui_hdr_srv_join(srv_name, chat_name, passwd);
}

int ui_hdr_srv_part(SrainChat *chat, const char *reason){
    const char *srv_name;
    const char *chat_name;

    srv_name = srain_chat_get_srv_name(chat);
    chat_name = srain_chat_get_name(chat);

    DBG_FR("srv_name: %s, chat_name: %s, reason: %s",
            srv_name, chat_name, reason);

    return _ui_hdr_srv_part(srv_name, chat_name, reason);
}

int ui_hdr_srv_send(SrainChat *chat, const char *msg){
    const char *chat_name;
    const char *srv_name;

    if (chat == NULL){
        ERR_FR("chat: NULL, msg: %s", msg);
        return -1;
    }

    srv_name = srain_chat_get_srv_name(chat);
    chat_name = srain_chat_get_name(chat);

    DBG_FR("srv_name: %s, chat_name: %s, msg: '%s'", srv_name, chat_name, msg);

    return _ui_hdr_srv_send(srv_name, chat_name, msg);
}
