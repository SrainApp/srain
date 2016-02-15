#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "ui.h"
#include "irc.h"
#include "irc_cmd.h"
#include "log.h"
#include "idles.h"

irc_t irc;

int srain_connect(const char *server, const char *alias){
    chan_name_t *chan;

    irc.alias = (char *)alias;
    irc.server = (char *)server;

    chan = malloc(sizeof(chan_name_t));
    strncpy(chan->name, alias, CHAN_LEN);
    ui_chan_add(chan);

    return irc_connect(&irc, server, "6666");
}

int srain_login(const char *nick){
    return irc_login(&irc, nick);
}

int srain_join(const char *chan){
    chan_name_t *chan2;

    chan2 = malloc(sizeof(chan_name_t));
    strncpy(chan2->name, chan, CHAN_LEN);
    ui_chan_add(chan2);

    if (irc_join_chan(&irc, chan) < 0){
        return -1;
    }
    return 0;
}

int srain_send(const char *chan, const char *msg){
    bubble_msg_t bmsg;

    LOG_FR("send message %s to %s", msg, chan);
    memset(&bmsg, 0, sizeof(bubble_msg_t));

    strncpy(bmsg.chan, chan, CHAN_LEN);
    strncpy(bmsg.msg, msg, MSG_LEN);
    get_cur_time(bmsg.time);

    ui_msg_send(&bmsg);
    irc_send(&irc, chan, msg);

    return 0;
}


void srain_recv(){
    irc_msg_t imsg;
    irc_msg_type_t type;

    LOG_FR("start listening in a new thread");

    for (;;){
        type = irc_recv(&irc, &imsg);

        if (type == IRCMSG_MSG){
            if (strcmp(imsg.command, "PRIVMSG") == 0){
                add_idle_ui_msg_recv(&imsg);
            }
            else if (strcmp(imsg.command, "TOPIC") == 0
                    || strcmp(imsg.command, RPL_TOPIC) == 0){
                add_idle_ui_chan_set_topic(&imsg);
            }
            else if (strcmp(imsg.command, "JOIN") == 0
                    || strcmp(imsg.command, "PART") == 0){
                add_idle_ui_msg_sys(&imsg);
            }
            else if (strcmp(imsg.command, RPL_NAMREPLY) == 0){
                add_idle_ui_online_list_init(&imsg);
            }
            else if (strcmp(imsg.command, "NOTICE") == 0
                    || strcmp(imsg.command, RPL_WELCOME) == 0
                    || strcmp(imsg.command, RPL_YOURHOST) == 0
                    || strcmp(imsg.command, RPL_CREATED) == 0
                    || strcmp(imsg.command, RPL_MOTD) == 0
                    || strcmp(imsg.command, RPL_ENDOFMOTD) == 0){
                add_idle_ui_msg_recv2(irc.alias, &imsg);
            }
            else if (strcmp(imsg.command, RPL_MYINFO) == 0
                    || strcmp(imsg.command, RPL_BOUNCE) == 0
                    || strcmp(imsg.command, RPL_LUSEROP) == 0
                    || strcmp(imsg.command, RPL_LUSERUNKNOWN) == 0
                    || strcmp(imsg.command, RPL_LUSERCHANNELS) == 0){
                add_idle_ui_msg_recv3(irc.alias, &imsg);
            } else {

            }
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
