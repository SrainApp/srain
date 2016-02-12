#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <time.h>
#include <string.h>
#include "ui.h"
#include "irc.h"
#include "irc_cmd.h"
#include "log.h"

irc_t irc;

static void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, 32, "%m-%d %H:%M", localtime(&curtime));
    timestr[31] = '\0';
}

int srain_connect(const char *server, const char *alias){
    irc.alias = (char *)alias;
    irc.server = (char *)server;

    ui_chat_add(alias, server);
    return irc_connect(&irc, server, "6666");
}

int srain_login(const char *nick){
    return irc_login(&irc, nick);
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
    bubble_msg_t bmsg;

    LOG_FR("send message %s to %s", msg, chan);

    get_cur_time(timestr);
    bmsg.time = timestr;
    bmsg.msg = (char *)msg;
    bmsg.nick = irc.nick;
    bmsg.chan = (char *)chan;
    bmsg.img = NULL;

    ui_msg_send(&bmsg);
    irc_send(&irc, chan, msg);
    return 0;
}


void srain_recv(){
    char timestr[32];
    int pool = 0;
    irc_msg_type_t type;
    bubble_msg_t bmsg_pool[40];
    irc_msg_t imsg_pool[40];

    bubble_msg_t *bmsg;

    irc_msg_t *imsg;

    LOG_FR("start listen in a new thread");

    for (;;){
        imsg = &imsg_pool[(pool++)%39];
        bmsg = &bmsg_pool[(pool++)%39];
        LOG_FR("bmsg locked");

        while (bmsg->locked);

        LOG_FR("bmsg unlocked");

        memset(imsg, 0, sizeof(irc_msg_t));
        memset(bmsg, 0, sizeof(bubble_msg_t));

        get_cur_time(timestr);
        bmsg->time = timestr;
        type = irc_recv(&irc, imsg);

        if (type == IRCMSG_MSG){
            if (strcmp(imsg->command, "PRIVMSG") == 0){
                bmsg->nick = imsg->nick;
                bmsg->chan = imsg->param[0];
                bmsg->msg = imsg->message;
            }
            else if (strcmp(imsg->command, "TOPIC") == 0
                    || strcmp(imsg->command, RPL_TOPIC) == 0){

            }
            else if (strcmp(imsg->command, "NOTICE") == 0
                    || strcmp(imsg->command, RPL_WELCOME) == 0
                    || strcmp(imsg->command, RPL_YOURHOST) == 0
                    || strcmp(imsg->command, RPL_CREATED) == 0){
                LOG_FR("NOTICE");
                bmsg->nick = imsg->servername;
                bmsg->chan = irc.alias;
                bmsg->msg = imsg->message;
            }
            else if (strcmp(imsg->command, RPL_MYINFO) == 0
                    || strcmp(imsg->command, RPL_BOUNCE) == 0
                    || strcmp(imsg->command, RPL_LUSEROP) == 0
                    || strcmp(imsg->command, RPL_LUSERUNKNOWN) == 0
                    || strcmp(imsg->command, RPL_LUSERCHANNELS) == 0){
                int i;
                char tmp[MSG_LEN];
                memset(tmp, 0, MSG_LEN);

                bmsg->chan = irc.alias;
                bmsg->nick = imsg->servername;
                // overflow?
                for (i = 1; i < imsg->nparam; i++){
                    strcat(tmp, imsg->param[i]);
                    strcat(tmp, " ");
                }
                strcat(tmp, imsg->message);
                strncpy(imsg->message, tmp, MSG_LEN);
                bmsg->msg = imsg->message;
            } else {
                bmsg->nick = imsg->servername;
                bmsg->chan = irc.alias;
                bmsg->msg = imsg->message;
            }

            bmsg->locked = 1;
            gdk_threads_add_idle((GSourceFunc)ui_msg_recv, bmsg);
            LOG_FR("idle added");
        }
    }
}
    int srain_listen(){
        g_thread_new(NULL, (void *)srain_recv, NULL);
        return 0;
    }

    void srain_close(){
        gtk_main_quit();
        irc_close(&irc);
    }
