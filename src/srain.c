#include <glib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "ui.h"
#include "irc.h"
#include "log.h"
#include "msg.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

IRC irc;

int srain_init(){
    irc_connect(&irc, "irc.freenode.net", "6666");
    irc_login(&irc, "srainbot2222");
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
   time_t curtime;
   MsgSend smsg;

   LOG_FR("send message %s to %s", msg, chan);
   time(&curtime);
   strftime(timestr, 32, "%m-%d %H:%M", localtime(&curtime));
   timestr[31] = '\0';

   smsg.time = timestr;
   smsg.msg = (char *)msg;
   smsg.nick = irc.nick;
   smsg.chan = (char *)chan;
   smsg.img = NULL;

   pthread_mutex_lock(&mutex);
   ui_msg_send(smsg);
   irc_send(&irc, chan, msg);
   pthread_mutex_unlock(&mutex);
   return 0;
}

void srain_recv(){
    MsgRecv rmsg = {.avatar = NULL, .img = NULL, .id = "unknown" };
    char timestr[32];
    time_t curtime;
    char irc_nick[NICK_LEN];
    char irc_msg[MSG_LEN];

    LOG_FR("start listen in a new thread");

    for (;;){
        if (irc_recv(&irc, irc_nick, irc_msg) == 0){
            LOG_FR("receive message %s from %s", irc_msg, irc_nick);
            pthread_mutex_lock(&mutex);
            time(&curtime);
            strftime(timestr, 32, "%m-%d %H:%M", localtime(&curtime));
            timestr[31] = '\0';
            rmsg.time = timestr;
            rmsg.nick = irc_nick;
            rmsg.chan = "#lasttest";
            rmsg.msg = irc_msg;
            LOG_FR("==1 %s", rmsg.chan);
            ui_msg_recv(rmsg);
            pthread_mutex_lock(&mutex);
        }
    }
}
int srain_listen(){
    pthread_t thread;

    pthread_create(&thread, NULL, (void *)srain_recv, NULL);
    return 0;
}
