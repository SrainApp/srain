/**
 * @file srain.c
 * @brief logic control layer of the whole application
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * This file gules UI module and IRC module together
 * and provides some abstract operations of a IRC client:
 *      connect to server, login as someone, join a channel,
 *      part from a channel, send message, receive message and etc.
 *
 */

#include <string.h>
#include <gtk/gtk.h>
#include "irc.h"
#include "irc_magic.h"
#include "ui.h"
#include "srain_window.h"
#include "log.h"

// return and stop spinner: call ui_busy(FALSE)
#define RET(x) while(1){ int tmp = x; ui_busy(FALSE); return tmp; }

irc_t irc;
GList *chan_list = NULL;    // list of GString
GThread *thread = NULL;

enum {
    SRAIN_UNCONNECTED,
    SRAIN_CONNECTING,
    SRAIN_CONNECTED,
    SRAIN_LOGINED
} stat = SRAIN_UNCONNECTED;

void srain_recv();
void _srain_connect(const char *server){
    if (stat != SRAIN_UNCONNECTED){
        ERR_FR("you have connected");
        return;
    }

    memset(&irc, 0, sizeof(irc_t));
    strncpy(irc.alias, server, CHAN_LEN);
    strncpy(irc.server, server, sizeof(irc.server) - 1);

    stat = SRAIN_CONNECTING;
    irc_connect(&irc, server, "6666");
    stat = SRAIN_CONNECTED;
    srain_recv();   // dead loop
}

int srain_connect(const char *server){
    ui_busy(TRUE);

    if (!thread) {
        thread = g_thread_new(NULL, (GThreadFunc)_srain_connect, (char *)server);
        while (stat != SRAIN_CONNECTED)
            while (gtk_events_pending()) gtk_main_iteration();
        RET(0);
    } else
        RET(-1);
}

int srain_login(const char *nick){
    ui_busy(TRUE);

    if (stat != SRAIN_CONNECTED){
        ERR_FR("NO SRAIN_CONNECTED");
        RET(-1);
    }

    chan_list = g_list_append(chan_list, "*server*");
    strncpy(irc.nick, nick, NICK_LEN);

    if (irc_reg(&irc, nick, "Srain", "EL PSY CONGRO") >= 0){
        stat = SRAIN_LOGINED;
        RET(0);
    }

    RET(-1);
}

int srain_join(const char *chan){
    GList *tmp;

    ui_busy(TRUE);

    if (stat != SRAIN_LOGINED){
        ERR_FR("NO SRAIN_LOGINED");
        RET(-1);
    }

    tmp = chan_list;
    while (tmp){
        if (strncmp(tmp->data, chan, CHAN_LEN) == 0){
            ERR_FR("channel already exist");
            RET(-1);
        }
        tmp = tmp->next;
    }

    irc_join(&irc, chan);
    RET(0);
}

int srain_part(const char *chan, const char *reason){
    GList *tmp;

    ui_busy(TRUE);
    if (!reason) reason = "Srain";

    tmp = chan_list;
    while (tmp){
        if (strncmp(tmp->data, chan, CHAN_LEN) == 0){
            irc_part(&irc, chan, reason);
            RET(0);
        }
        tmp = tmp->next;
    }

    ERR_FR("no such channel");
    RET(-1);
}

int srain_send(const char *chan, const char *msg){
    ui_busy(TRUE);
    LOG_FR("send message '%s' to %s", msg, chan);

    ui_msg_send(chan, msg);
    RET(irc_msg(&irc, chan, msg));
}

/* GSourceFunc */
gboolean srain_idles(irc_msg_t *imsg){
    /* Message */
    if (strcmp(imsg->command, "PRIVMSG") == 0){
        if (imsg->nparam != 1) goto bad;
        ui_msg_recv(imsg->param[0], imsg->nick, imsg->nick, imsg->message);
    }

    /* Topic */
    else if (strcmp(imsg->command, "TOPIC") == 0){
        if (imsg->nparam != 1) goto bad;
        ui_chan_set_topic(imsg->param[0], imsg->message);
    }
    else if (strcmp(imsg->command, RPL_TOPIC) == 0){
        if (imsg->nparam == 2) ui_chan_set_topic(imsg->param[1], imsg->message);
    }

    /* JOIN & PART */
    else if (strcmp(imsg->command, "JOIN") == 0){
        if (imsg->nparam != 1) goto bad;
        if (strncmp(imsg->nick, irc.nick, NICK_LEN) == 0){
            ui_chan_add(imsg->param[0]);
            chan_list = g_list_append(chan_list, strdup(imsg->param[0]));
        }
        else ui_chan_online_list_add(imsg->param[0], imsg->nick);
        ui_msg_sysf(imsg->param[0], "%s join %s", imsg->nick, imsg->param[0]);
    }
    else if (strcmp(imsg->command, "PART") == 0){
        if (strncmp(imsg->nick, irc.nick, NICK_LEN) == 0){
            GList *tmp = chan_list;
            while (tmp){
                if (strncmp(imsg->param[0], tmp->data, CHAN_LEN) == 0){
                    LOG_FR("PART %s - %s", imsg->param[0], tmp->data);
                    free(tmp->data);
                    chan_list = g_list_remove(chan_list, tmp->data);
                    ui_chan_rm(imsg->param[0]);
                }
                tmp = tmp->next;
            }
        }
        else{
            ui_chan_online_list_rm(imsg->param[0], imsg->nick);
            ui_msg_sysf(imsg->param[0], "%s leave %s", imsg->nick, imsg->param[0]);
        }
    }
    else if (strcmp(imsg->command, "QUIT") == 0){
        // if (imsg->nparam == 1) ui_chan_onlinelist_add(imsg->param[0], imsg->nick);
    }

    /* NICK (someone change his name) */
    else if (strcmp(imsg->command, "NICK") == 0){
        if (imsg->nparam != 0) goto bad;
        ui_msg_sysf(NULL, "%s is now known as %s", imsg->nick, imsg->message);
        // ui_chan_onlinelist_rm("*", irc->nick);
        // ui_chan_onlinelist_add("*", irc->nick);
        if (strncmp(irc.nick, imsg->nick, NICK_LEN) == 0)
            strncpy(irc.nick, imsg->message, NICK_LEN);
    }

    /* Names (Channel name list) */
    else if (strcmp(imsg->command, RPL_NAMREPLY) == 0){
        if (imsg->nparam != 3) goto bad;
        char *nickptr = strtok(imsg->message, " ");
        while (nickptr){
            ui_chan_online_list_add(imsg->param[2], nickptr);
            nickptr = strtok(NULL, " ");
        }
    }

    /* NOTICE */
    else if (strcmp(imsg->command, "NOTICE") == 0){
        if (imsg->nparam != 1) goto bad;
        if (strcmp(imsg->param[0], "*") == 0)
            ui_msg_recv_broadcast(chan_list, imsg->servername, irc.server, imsg->message);
        else
            ui_msg_recv(imsg->param[0], imsg->servername, irc.server, imsg->message);
    }

    /* Other Server Mesage (receive when login) */
    else if (strcmp(imsg->command, RPL_WELCOME) == 0
            || strcmp(imsg->command, RPL_YOURHOST) == 0
            || strcmp(imsg->command, RPL_CREATED) == 0
            || strcmp(imsg->command, RPL_MOTD) == 0
            || strcmp(imsg->command, RPL_ENDOFMOTD) == 0
            || strcmp(imsg->command, RPL_MYINFO) == 0
            || strcmp(imsg->command, RPL_BOUNCE) == 0
            || strcmp(imsg->command, RPL_LUSEROP) == 0
            || strcmp(imsg->command, RPL_LUSERUNKNOWN) == 0
            || strcmp(imsg->command, RPL_LUSERCHANNELS) == 0){
        /* regard a server message as a PRVIMSG message
         * let nick = servername
         *     msg = param[1..n] + message
         */
        int i = 1;
        char tmp[MSG_LEN];
        memset(tmp, 0, sizeof(tmp));
        while (i < imsg->nparam){
            strcat(tmp, imsg->param[i++]);
            strcat(tmp, " ");
        }
        strcat(tmp, imsg->message);
        if (strlen(tmp) >= MSG_LEN){
            ERR_FR("message too long");
            goto bad;
        }
        ui_msg_recv("*server*", imsg->servername, irc.server, tmp);
    }

    /* RPL_ERROR */
    else if (imsg->command[0] == '4'){
        ui_msg_sys(NULL, imsg->message);
    }

    /* unsupported message */
    else {
        ERR_FR("unsupported message");
        LOG("\tcommand: %s\n", imsg->command);
        LOG("\tmessage: %s\n", imsg->message);
    }

    free(imsg);
    return FALSE;   // you must return FALSE when execute normally

bad:
    ERR_FR("wrong paramater count");
    free(imsg);
    return FALSE;   // you must return FALSE when execute normally
}

/* this function work in listening thread */
void srain_recv(){
    irc_msg_t *imsg;
    irc_msg_type_t type;

    LOG_FR("start listening in a new thread");

    for (;;){
        imsg = calloc(1, sizeof(irc_msg_t));
        type = irc_recv(&irc, imsg);

        if (type == IRCMSG_SCKERR){
            ERR_FR("socket error, listen thread stop, connection close");
            irc_close(&irc);
            return;
        }
        if (type == IRCMSG_MSG){
            // let main loop process data
            gdk_threads_add_idle((GSourceFunc)srain_idles, imsg);
        } else {
            free(imsg);
        }
    }
}

void srain_close(){
    gtk_main_quit();
    irc_quit(&irc, "EL PSY CONGRO");
    irc_close(&irc);
}

int srain_cmd(const char *chan, char *cmd){
    // ui_busy(TRUE);
    /* TODO
     * ignore
     * whois
     * help
     * names
     * */
    ui_msg_sysf(chan, "command: %s", cmd);

    if (strncmp(cmd, "/connect", 8) == 0){
        char *server = strtok(cmd + 8, " ");
        if (server) return srain_connect(server);
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
        else return srain_part(ui_chan_get_cur_name(), NULL);
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
        if (msg){
            ui_msg_send(NULL, msg);
            return irc_action(&irc, ui_chan_get_cur_name(), msg);
        }
    }
    else if (strncmp(cmd, "/nick", 5) == 0){
        char *nick = strtok(cmd + 5, " ");
        if (nick){
            /* irc->nick will be modified when recv
             * NICK command from server */
            return irc_nick(&irc, nick);
        }
    } else {
        ui_msg_sysf(chan, "%s: unsupported command", cmd);
        return -1;
    }

    ui_msg_sysf(chan, "missing parameter");
    return -1;
}
