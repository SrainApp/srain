#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "ui.h"
#include "irc.h"
#include "irc_cmd.h"
#include "async.h"
#include "log.h"

irc_t irc;

int srain_listen();
int srain_connect(const char *server, const char *alias){
    if (irc.fd){
        async_call_ui_msg_sys(ui_chan_get_cur(), "you have connected");
        return -1;
    }

    strncpy(irc.alias, alias, CHAN_LEN);
    strncpy(irc.server, server, sizeof(irc.server) - 1);

    irc.bufptr = 0;
    memset(irc.chans, 0, sizeof(irc.chans));

    ui_chan_add(alias);
    irc_connect(&irc, server, "6666");
    srain_listen();
    return 0;
}

int srain_login(const char *nick){
    if (!irc.fd){
        async_call_ui_msg_sys(ui_chan_get_cur(), "you are no connect to any server");
        return -1;
    }
    if (strlen(irc.nick) != 0){
        async_call_ui_msg_sys(ui_chan_get_cur(), "you have logined");
        return -1;
    }
    strncpy(irc.nick, nick, NICK_LEN);

    return irc_reg(&irc, nick, "Srain", "EL PSY CONGRO");
}

int srain_join(const char *chan){
    int i, empty = -1;

    for (i = 0; i < CHAN_NUM; i++){
        if (strncmp(irc.chans[i], chan, CHAN_LEN) == 0){
            ERR_FR("channels already exist");
            return -1;
        }
        if (strlen(irc.chans[i]) == 0)
            empty = (empty != -1 && empty < i) ? empty : i;
    }
    if (empty != -1){
        irc_join(&irc, chan);
        strncpy(irc.chans[empty], chan, CHAN_LEN);
        ui_chan_add(chan);
        return 0;
    }

    ERR_FR("channels list is full");
    return -1;
}

int srain_part(const char *chan, const char *reason){
    int i;
    if (!reason) reason = "Srain";

    for (i = 0; i < CHAN_NUM; i++){
        if (strncmp(irc.chans[i], chan, CHAN_LEN) == 0){
            irc_part(&irc, chan, reason);
            ui_chan_rm(chan);
            memset(irc.chans[i], 0, CHAN_LEN);
            return 0;
        }
    }

    ERR_FR("no such channel %s", chan);
    return -1;
}

int srain_send(const char *chan, const char *msg){
    LOG_FR("send message \"%s\" to %s", msg, chan);

    ui_msg_send(chan, msg, NULL);
    return irc_msg(&irc, chan, msg);
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
                async_recv_msg_normal(&imsg);
            }
            else if (strcmp(imsg.command, "TOPIC") == 0
                    || strcmp(imsg.command, RPL_TOPIC) == 0){
                async_set_topic(&imsg);
            }
            else if (strcmp(imsg.command, "JOIN") == 0
                    || strcmp(imsg.command, "PART") == 0
                    || strcmp(imsg.command, "QUIT") == 0){
                /* we receive one QUIT messge when a people quit, but we should remove this people from
                 * all channels he has join in, TODO
                 */
                if (strncmp(imsg.nick, irc.nick, NICK_LEN) != 0){
                    async_join_part(&imsg);
                }
            }
            else if (strcmp(imsg.command, RPL_NAMREPLY) == 0){
                async_online_list_init(&imsg);
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
                async_recv_msg_server(irc.alias, &imsg);
            } else if (imsg.command[0] == '4'){
                /* RPL_ERROR */
                async_call_ui_msg_sys(ui_chan_get_cur(), imsg.message);
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
     * ignore
     * whois
     * help
     * names
     * */
    if (strncmp(cmd, "/connect", 8) == 0){
        char *server = strtok(cmd + 8, " ");
        char *alias= strtok(NULL, " ");
        if (server && alias) return srain_connect(server, alias);
    }
    else if (strncmp(cmd, "/login", 6) == 0){
        char *nick = strtok(cmd + 6, " ");
        if (nick) return srain_login(nick);
    }
    else if (strncmp(cmd, "/join", 5) == 0){
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
            strncpy(irc.nick, nick, NICK_LEN);
        }
    } else {
        async_call_ui_msg_sys(chan, "unsupported command");
        return -1;
    }

    char errmsg[128];
    snprintf(errmsg, 127, "`%s` missing parameter", cmd);
    async_call_ui_msg_sys(chan, errmsg);
    return -1;
}

