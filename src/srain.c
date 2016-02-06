#include <glib.h>
#include <gdk/gdk.h>
#include <time.h>
#include <string.h>
#include "ui.h"
#include "irc.h"
#include "log.h"
#include "msg.h"

IRC irc;

static void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, 32, "%m-%d %H:%M", localtime(&curtime));
    timestr[31] = '\0';
}


int srain_init(){
    irc_connect(&irc, "irc.freenode.net", "6666");
    irc_login(&irc, "srainbot222");
    return 0;
}

int srain_join(const char *chan){
    if (ui_chat_add(chan, "") < 0){
        return -1;
    }
    if (irc_join_chan(&irc, chan) < 0){
        return -1;
    }
    return 0;
}

int srain_send(const char *chan, const char *msg){
    char timestr[32];
    MsgSend smsg;

    LOG_FR("send message %s to %s", msg, chan);

    get_cur_time(timestr);
    smsg.time = timestr;
    smsg.msg = (char *)msg;
    smsg.nick = irc.nick;
    smsg.chan = (char *)chan;
    smsg.img = NULL;

    ui_msg_send(smsg);
    irc_send(&irc, chan, msg);
    return 0;
}

void srain_recv(){
    MsgRecv rmsg = {
        .avatar = NULL,
        .img = NULL,
        .id = "unknown",
        .time = "unknown",
        .nick = "unknown",
        .chan = "unknown",
        .msg = "unknown"
    };
    char timestr[32];
    char cmd[32];
    char msg[MSG_LEN];
    char nick[NICK_LEN];
    char chan[CHAN_LEN];

    LOG_FR("start listen in a new thread");

    for (;;){
        if (irc_recv(&irc, nick, chan, cmd, msg) == 0){
            // LOG_FR("receive message %s from %s", irc_msg, irc_nick);
            get_cur_time(timestr);
            rmsg.time = timestr;
            rmsg.nick = nick;
            rmsg.chan = chan;
            rmsg.msg = msg;
            // gdk_threads_add_idle or gdk_idle_add ?
            g_idle_add((GSourceFunc)ui_msg_recv, &rmsg);
            LOG_FR("idle added");
        }
    }
}

int srain_listen(){
    g_thread_new(NULL, (void *)srain_recv, NULL);
    return 0;
}
