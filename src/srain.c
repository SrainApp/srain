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
        while (stat != SRAIN_CONNECTED){
            while (gtk_events_pending()) gtk_main_iteration();
        }
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

    strncpy(irc.nick, nick, NICK_LEN);

    if (irc_reg(&irc, nick, "Srain", "EL PSY CONGRO") >= 0){
        stat = SRAIN_LOGINED;
        RET(0);
    }

    RET(-1);
}

int srain_join(const char *chan){
    int i, empty = -1;

    ui_busy(TRUE);

    if (stat != SRAIN_LOGINED){
        ERR_FR("NO SRAIN_LOGINED");
        RET(-1);
    }

    for (i = 0; i < CHAN_NUM; i++){
        if (strncmp(irc.chans[i], chan, CHAN_LEN) == 0){
            ERR_FR("channels already exist");
            RET(-1);
        }
        if (strlen(irc.chans[i]) == 0)
            empty = (empty != -1 && empty < i) ? empty : i;
    }
    if (empty != -1){
        irc_join(&irc, chan);
        strncpy(irc.chans[empty], chan, CHAN_LEN);
        ui_chan_add(chan);
        RET(0);
    }

    ERR_FR("channels list is full");
    RET(-1);
}

int srain_part(const char *chan, const char *reason){
    int i;
    if (!reason) reason = "Srain";

    ui_busy(TRUE);

    for (i = 0; i < CHAN_NUM; i++){
        if (strncmp(irc.chans[i], chan, CHAN_LEN) == 0){
            irc_part(&irc, chan, reason);
            ui_chan_rm(chan);
            memset(irc.chans[i], 0, CHAN_LEN);
            RET(0);
        }
    }

    ERR_FR("no such channel %s", chan);
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
    if (strcmp(imsg->command, "PRIVMSG") == 0){
        if (imsg->nparam != 1) goto bad;
        ui_msg_recv(imsg->param[0], imsg->nick, imsg->nick, imsg->message);
    }
    else if (strcmp(imsg->command, "TOPIC") == 0
            || strcmp(imsg->command, RPL_TOPIC) == 0){
        // async_set_topic(&imsg);
    }
    else if (strcmp(imsg->command, "JOIN") == 0
            || strcmp(imsg->command, "PART") == 0
            || strcmp(imsg->command, "QUIT") == 0){
        /* we receive one QUIT messge when a people quit, but we should remove this people from
         * all channels he has join in, TODO
         */
        if (strncmp(imsg->nick, irc.nick, NICK_LEN) != 0){
            // async_join_part(&imsg);
        }
    }
    else if (strcmp(imsg->command, RPL_NAMREPLY) == 0){
        // async_online_list_init(&imsg);
    }
    else if (strcmp(imsg->command, "NOTICE") == 0
            || strcmp(imsg->command, RPL_WELCOME) == 0
            || strcmp(imsg->command, RPL_YOURHOST) == 0
            || strcmp(imsg->command, RPL_CREATED) == 0
            || strcmp(imsg->command, RPL_MOTD) == 0
            || strcmp(imsg->command, RPL_ENDOFMOTD) == 0
            || strcmp(imsg->command, RPL_MYINFO) == 0
            || strcmp(imsg->command, RPL_BOUNCE) == 0
            || strcmp(imsg->command, RPL_LUSEROP) == 0
            || strcmp(imsg->command, RPL_LUSERUNKNOWN) == 0
            || strcmp(imsg->command, RPL_LUSERCHANNELS) == 0){
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
    } else if (imsg->command[0] == '4'){
        /* RPL_ERROR */
        // async_call_ui_msg_sys(ui_chan_get_cur(), imsg->message);
    } else {
        ERR_FR("unsupported message");
    }

bad:
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
    ui_busy(TRUE);
    /* TODO
     * ignore
     * whois
     * help
     * names
     * */
    if (strncmp(cmd, "/connect", 8) == 0){
        char *server = strtok(cmd + 8, " ");
        if (server) srain_connect(server);
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
        // if (msg) return irc_action(&irc, ui_chan_get_cur(), msg);
    }
    else if (strncmp(cmd, "/nick", 5) == 0){
        char *nick = strtok(cmd + 5, " ");
        if (nick){
            return irc_nick(&irc, nick);
            // TODO null?
            strncpy(irc.nick, nick, NICK_LEN);
        }
    } else {
        // async_call_ui_msg_sys(chan, "unsupported command");
        return -1;
    }

    char errmsg[128];
    // snprintf(errmsg, 127, "`%s` missing parameter", cmd);
    // async_call_ui_msg_sys(chan, errmsg);
    RET(-1);
}
