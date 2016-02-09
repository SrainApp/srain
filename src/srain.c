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


int srain_login(const char *nick){
    irc_connect(&irc, "irc.freenode.net", "6666");
    ui_chat_add("irc.freenode.net", "");
    irc_login(&irc, nick);

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

    IRCMsg ircmsg_pool[20];
void srain_recv(){
    int poolptr = 0;
    char timestr[32];
    MsgRecv rmsg_pool[20];
    MsgRecv *rmsg;
    IRCMsg *ircmsg;
    IRCMsgType msgtype;

    LOG_FR("start listen in a new thread");

    for (;;){
        ircmsg = &ircmsg_pool[(poolptr++)%19];
        rmsg = &rmsg_pool[(poolptr++)%19];
        memset(ircmsg, 0, sizeof(IRCMsg));
        memset(rmsg, 0, sizeof(MsgRecv));
        get_cur_time(timestr);
        rmsg->time = timestr;
        msgtype = irc_recv(&irc, ircmsg);
        if (msgtype == IRCMSG_MSG_NORMAL){
            if (strcmp(ircmsg->command, "PRIVMSG") == 0){
                rmsg->nick = ircmsg->nick;
                rmsg->chan = ircmsg->param[0];
            } else continue;
        }
        else if (msgtype == IRCMSG_MSG_SERVER){
            rmsg->nick = ircmsg->servername;
            rmsg->chan = "irc.freenode.net";
        } else continue;
        rmsg->msg = ircmsg->message;
        LOG_FR("{\n%s}", rmsg->msg);
        // gdk_threads_add_idle or gdk_idle_add ?
        g_idle_add((GSourceFunc)ui_msg_recv, rmsg);
        LOG_FR("idle added");
    }
}

int srain_listen(){
    g_thread_new(NULL, (void *)srain_recv, NULL);
    return 0;
}
