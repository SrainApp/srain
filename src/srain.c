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
    return irc_reg(&irc, nick, NULL, NULL);
}

int srain_join(const char *chan){
    chan_name_t *chan2;

    chan2 = calloc(1, sizeof(chan_name_t));
    strncpy(chan2->name, chan, CHAN_LEN);
    ui_chan_add(chan2);

    return irc_join(&irc, chan);
}

int srain_part(const char *chan, const char *reason){
    chan_name_t *chan2;
    if (!reason) reason = "";

    chan2 = calloc(1, sizeof(chan_name_t));
    strncpy(chan2->name, chan, CHAN_LEN);

    if (irc_part(&irc, chan, reason) != -1){
        ui_chan_rm(chan2);
        return 0;
    }

    return -1;
}

int srain_send(const char *chan, const char *msg){
    bubble_msg_t bmsg;

    LOG_FR("send message \"%s\" to %s", msg, chan);
    memset(&bmsg, 0, sizeof(bubble_msg_t));

    strncpy(bmsg.chan, chan, CHAN_LEN);
    strncpy(bmsg.msg, msg, MSG_LEN);
    get_cur_time(bmsg.time);

    ui_msg_send(&bmsg);
    irc_msg(&irc, chan, msg);

    return 0;
}


void srain_recv(){
    irc_msg_t imsg;
    irc_msg_type_t type;

    LOG_FR("start listening in a new thread");

    for (;;){
        memset(&imsg, 0, sizeof(irc_msg_t));
        type = irc_recv(&irc, &imsg);

        if (type == IRCMSG_MSG){
            if (strcmp(imsg.command, "PRIVMSG") == 0){
                idles_msg_normal(&imsg);
            }
            else if (strcmp(imsg.command, "TOPIC") == 0
                    || strcmp(imsg.command, RPL_TOPIC) == 0){
                idles_topic(&imsg);
            }
            else if (strcmp(imsg.command, "JOIN") == 0
                    || strcmp(imsg.command, "PART") == 0
                    || strcmp(imsg.command, "QUIT") == 0){
                /* we receive one QUIT messge when a people quit, but we should remove this people from
                 * all channels he has join in, TODO
                 */
                if (strncmp(imsg.nick, irc.nick, NICK_LEN) != 0){
                    idles_join_part(&imsg);
                }
            }
            else if (strcmp(imsg.command, RPL_NAMREPLY) == 0){
                idles_names(&imsg);
            }
            else if (strcmp(imsg.command, "NOTICE") == 0
                    || strcmp(imsg.command, RPL_WELCOME) == 0
                    || strcmp(imsg.command, RPL_YOURHOST) == 0
                    || strcmp(imsg.command, RPL_CREATED) == 0
                    || strcmp(imsg.command, RPL_MOTD) == 0
                    || strcmp(imsg.command, RPL_ENDOFMOTD) == 0
                    || strcmp(imsg.command, RPL_MYINFO) == 0
                    || strcmp(imsg.command, RPL_BOUNCE) == 0
                    || strcmp(imsg.command, RPL_LUSEROP) == 0
                    || strcmp(imsg.command, RPL_LUSERUNKNOWN) == 0
                    || strcmp(imsg.command, RPL_LUSERCHANNELS) == 0){
                idles_msg_server(irc.alias, &imsg);
            } else if (imsg.command[0] == '4'){
                add_idle_ui_msg_sys("", imsg.message);
            }
        }
    }
}

int srain_listen(){
    g_thread_new(NULL, (void *)srain_recv, NULL);
    return 0;
}


int srain_cmd(const char *chan, char *cmd){
    /* TODO
     * me
     * msg
     * nick
     * ping
     * quit
     * ignore
     * whois
     * help
     * names
     * */
    if (strncmp(cmd, "/join", 5) == 0){
        // TODO
        char *jchan = strtok(cmd + 5, " ");
        if (jchan) return srain_join(jchan);
    } 
    else if (strncmp(cmd, "/part", 5) == 0){
        char *pchan = strtok(cmd + 5, " ");
        if (pchan) return srain_part(pchan, "Srain 1.0");
    } else {
        add_idle_ui_msg_sys(chan, "unsupported command");
        return -1;
    }

    add_idle_ui_msg_sys(chan, "missing channel");
    return -1;
}

void srain_close(){
    gtk_main_quit();
    irc_close(&irc);
}
