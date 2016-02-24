#include <gtk/gtk.h>
#include <srain_window.h>
#include <srain_chan.h>
#include <srain_msg.h>

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

void ui_msg_sys(const char *chan_name, const char *msg){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(win, chan_name);
    if (chan){
        srain_chan_sys_msg_add(chan, msg);
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

void ui_busy(gboolean is_busy){
    srain_window_spinner_toggle(win, is_busy);
}
