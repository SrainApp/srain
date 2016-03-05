/**
 * @file ui.c
 * @brief UI module interface
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include <string.h>
#include "ui.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_msg.h"

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
    srain_chan_set_topic(chan, topic);
}

void ui_chan_online_list_add(const char *chan_name, const char *name, int is_init){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    srain_chan_online_list_add(chan, name, is_init);
}

void ui_chan_online_list_rm(const char *chan_name, const char *name, const char *resaon){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    srain_chan_online_list_rm(chan, name, resaon);
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
        srain_chan_online_list_rename(chan, old_name, new_name);
        chans = chans->next;
    }
}

const char* ui_chan_get_cur_name(){
    SrainChan *chan;

    chan = srain_window_get_cur_chan(win);

    if (chan) return gtk_widget_get_name(GTK_WIDGET(chan));
    return NULL;
}

void ui_msg_sys(const char *chan_name, sys_msg_type_t type, const char *msg){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);

    if (chan){
        srain_chan_sys_msg_add(chan, type, msg);
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
        srain_chan_send_msg_add(chan, msg, NULL);
    }
}

void ui_msg_recv(const char *chan_name, const char *nick, const char *id,
        const char *msg){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (chan){
        srain_chan_recv_msg_add(chan, nick, id, msg, NULL);
    }
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
