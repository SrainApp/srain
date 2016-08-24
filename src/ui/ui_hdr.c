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
#include "srain_chan.h"
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
 *      if NULL, command echo may appear in the current SrainChan
 * @param cmd the command you want to execute
 *
 * @return 0 if successed, -1 if failed
 */
int ui_hdr_srv_cmd(SrainChan *chan, char *cmd, int block){
    int res;
    char *cmd2;
    const char *srv_name;
    const char *chan_name;

    cmd2 = strdup(cmd);

    if (chan == NULL){
        chan = srain_window_get_cur_chan(srain_win);
    }

    if (chan){
        srv_name = srain_chan_get_srv_name(chan);
        chan_name = srain_chan_get_name(chan);
    } else {
        srv_name = chan_name = NULL;
    }

    DBG_FR("srv_name: %s, chan_name: %s, cmd: '%s', block: %d",
            srv_name, chan_name, cmd, block);
    res = _ui_hdr_srv_cmd(srv_name, chan_name, cmd2, block);

    free(cmd2);
    return res;
}

/**
 * @brief join a channel of current server
 *
 * @param chan_name
 */
int ui_hdr_srv_join(const char *chan_name, const char *passwd){
    const char *srv_name;
    SrainChan *chan;

    chan = srain_window_get_cur_chan(srain_win);
    srv_name = srain_chan_get_srv_name(chan);

    DBG_FR("srv_name: %s, chan_name: %s, passwd: %s",
            srv_name, chan_name, passwd);

    return _ui_hdr_srv_join(srv_name, chan_name, passwd);
}

int ui_hdr_srv_part(SrainChan *chan, const char *reason){
    const char *srv_name;
    const char *chan_name;

    srv_name = srain_chan_get_srv_name(chan);
    chan_name = srain_chan_get_name(chan);

    DBG_FR("srv_name: %s, chan_name: %s, reason: %s",
            srv_name, chan_name, reason);

    return _ui_hdr_srv_part(srv_name, chan_name, reason);
}

int ui_hdr_srv_send(SrainChan *chan, const char *msg){
    const char *chan_name;
    const char *srv_name;

    if (chan == NULL){
        ERR_FR("chan: NULL, msg: %s", msg);
        return -1;
    }

    srv_name = srain_chan_get_srv_name(chan);
    chan_name = srain_chan_get_name(chan);

    DBG_FR("srv_name: %s, chan_name: %s, msg: '%s'", srv_name, chan_name, msg);

    return _ui_hdr_srv_send(srv_name, chan_name, msg);
}
