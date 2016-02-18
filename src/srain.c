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
    if (!reason) reason = "Srain";

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
                /* RPL_ERROR */
                add_idle_ui_msg_sys(ui_chan_get_cur(), imsg.message);
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
    irc_quit(&irc, "EL PSY CONGRO");
    irc_close(&irc);
}

int srain_cmd(const char *chan, char *cmd){
    /* TODO
     * nick
     * ping
     * quit
     * ignore
     * whois
     * help
     * names
     * */
    if (strncmp(cmd, "/join", 5) == 0){
        char *jchan = strtok(cmd + 5, " ");
        if (jchan) return srain_join(jchan);
    } 
    else if (strncmp(cmd, "/part", 5) == 0){
        char *pchan = strtok(cmd + 5, " ");
        if (pchan) return srain_part(pchan, NULL);
    }
    else if (strncmp(cmd, "/quit", 5) == 0){
        srain_close();
    }
    else if (strncmp(cmd, "/msg", 4) == 0){
        char *to = strtok(cmd + 4, " ");
        char *msg = strtok(NULL, " ");
        if (to && msg) return srain_send(to, msg);
    }
    else if (strncmp(cmd, "/me", 3) == 0){
        char *msg = strtok(cmd + 3, " ");
        if (msg) return irc_action(&irc, ui_chan_get_cur(), msg);
    }
    else if (strncmp(cmd, "/nick", 5) == 0){
        char *nick = strtok(cmd + 5, " ");
        if (nick){
            return irc_nick(&irc, nick);
            // TODO null?
            irc.nick =  nick;
        }
    } else {
        add_idle_ui_msg_sys(chan, "unsupported command");
        return -1;
    }

    add_idle_ui_msg_sys(chan, "missing parameter");
    return -1;
}

