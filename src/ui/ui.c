/**
 * @file ui.c
 * @brief UI module's interface
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * this file offter some funcions to control/update
 * UI, you must invoke ui_init() firstly to get it knows
 * which SrainWindow to control.
 *
 * other module control UI module via this file, DO NOT
 * call any funcion in ui/xxx.c directly (of course apart
 * from this file)
 *
 * most UI operations return nothing and DO NO deal
 * with any exception, so functions in this file do it.
 *
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>
#include "meta.h"
#include "ui.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_msg.h"
#include "srain_stack_sidebar.h"
#include "log.h"

SrainWindow *win;

void ui_init(SrainWindow *swin){
    win = swin;
}

void ui_chan_add(const char *chan_name){
    srain_window_add_chan(win, chan_name);
}

void ui_chan_rm(const char *chan_name){
    srain_window_rm_chan(win, chan_name);
}

void ui_chan_set_topic(const char *chan_name, const char *topic){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (chan){
        srain_chan_set_topic(chan, topic);
    }
}

void ui_chan_online_list_add(const char *chan_name, const char *name, int is_init){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (chan){
        srain_chan_online_list_add(chan, name, is_init);
    }
}

void ui_chan_online_list_rm(const char *chan_name, const char *name, const char *resaon){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (chan){
        srain_chan_online_list_rm(chan, name, resaon);
    }
}


/**
 * @brief ui_chan_online_list_rm_broadcast
 *
 * called when recvied QUIT message
 *
 * @param chans
 * @param name
 * @param resaon
 */
void ui_chan_online_list_rm_broadcast(GList *chans, const char *name, const char *reason){
    while (chans){
        ui_chan_online_list_rm(chans->data, name, reason);
        chans = chans->next;
    }
}

void ui_chan_online_list_rename_broadcast(GList *chans, const char *old_name, const char *new_name){
    SrainChan *chan;

    while (chans){
        chan = srain_window_get_chan_by_name(win, chans->data);
        if (chan){
            srain_chan_online_list_rename(chan, old_name, new_name);
        }
        chans = chans->next;
    }
}

const char* ui_chan_get_cur_name(){
    SrainChan *chan;

    chan = srain_window_get_cur_chan(win);

    if (chan) return gtk_widget_get_name(GTK_WIDGET(chan));
    return NULL;
}

 /* if `chan_name` doesn't exist, send this msg to current SrainChan */
void ui_msg_sys(const char *chan_name, sys_msg_type_t type, const char *msg){
    SrainChan *chan;

    LOG_FR("chan_name %s msg %s", chan_name, msg);

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (!chan){
        ERR_FR("no such chan %s", chan_name);
        chan = srain_window_get_cur_chan(win);
        if (!chan) {
            ERR_FR("no current chan");
            return;
        }
    }

    srain_chan_sys_msg_add(chan, type, msg);

    if (type == SYS_MSG_ACTION){
        srain_window_stack_sidebar_update(win, chan, "", msg);
    }
}

void ui_msg_sysf(const char *chan_name, sys_msg_type_t type, const char *fmt, ...){
    char msg[512];
    va_list args;

    if (strlen(fmt) != 0 ){
        // Format the data
        va_start(args, fmt);
        vsnprintf(msg, sizeof (msg), fmt, args);
        va_end(args);

        ui_msg_sys(chan_name, type, msg);
    }
}

void ui_msg_sysf_broadcast(GList *chans, sys_msg_type_t type, const char *fmt, ...){
    char msg[512];
    va_list args;

    if (strlen(fmt) != 0 ){
        // Format the data
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);
    }

    while (chans){
        ui_msg_sys(chans->data, type, msg);
        chans = chans->next;
    }
}

void ui_msg_send(const char *chan_name, const char *msg){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (chan){
        srain_chan_send_msg_add(chan, msg);
        srain_window_stack_sidebar_update(win, chan, "YOU", msg);
    }
}

 /* if `chan_name` doesn't exist, send this msg to a
 * special SrainChan named `META_SERVER`
 */
void ui_msg_recv(const char *chan_name, const char *nick, const char *id,
        const char *msg){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (!chan){
        ERR_FR("no such chan %s", chan_name);
        chan = srain_window_get_chan_by_name(win, META_SERVER);
    }

    srain_chan_recv_msg_add(chan, nick, id, msg);
    srain_window_stack_sidebar_update(win, chan, nick, msg);
}

void ui_msg_recv_broadcast(GList *chans, const char *nick, const char *id,
        const char *msg){
    while (chans){
        ui_msg_recv(chans->data, nick, id, msg);
        chans = chans->next;
    }
}

void ui_busy(gboolean is_busy){
    srain_window_spinner_toggle(win, is_busy);
}
