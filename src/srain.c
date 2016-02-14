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
    strftime(timestr, TIME_LEN, "%m-%d %H:%M", localtime(&curtime));
    timestr[TIME_LEN-1] = '\0';
}

/* transfer a standard PRIVMSG message to bubble_msg_t,
 * then call `ui_msg_recv`
 */
static void add_idle_ui_msg_recv(const irc_msg_t *imsg){
    bubble_msg_t *bmsg;

    if (imsg->nparam != 1){
        ERR_FR("wrong param");
        return;
    }

    bmsg = calloc(1, sizeof(bubble_msg_t));
    get_cur_time(bmsg->time);
    strncpy(bmsg->nick, imsg->nick, NICK_LEN);
    strncpy(bmsg->chan, imsg->param[0], CHAN_LEN);
    strncpy(bmsg->msg, imsg->message, MSG_LEN);

    gdk_threads_add_idle((GSourceFunc)ui_msg_recv, bmsg);
    LOG_FR("idle added");
    return;
}

/* transfer a server message to bubble_msg_t,
 * let nick = servername
 *     msg = message
 *     chan = irc server's alias
 * then call `ui_msg_recv`
 */
static void add_idle_ui_msg_recv2(const irc_msg_t *imsg){
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    get_cur_time(bmsg->time);
    strncpy(bmsg->nick, imsg->servername, SERVER_LEN);
    strncpy(bmsg->chan, irc.alias, CHAN_LEN);
    strncpy(bmsg->msg, imsg->message, MSG_LEN);

    gdk_threads_add_idle((GSourceFunc)ui_msg_recv, bmsg);
    LOG_FR("idle added");
    return;
}

/* transfer a server message to bubble_msg_t,
 * let nick = servername
 *     msg = param[1..n] + message
 *     chan = irc server's alias
 * then call `ui_msg_recv`
 */
static void add_idle_ui_msg_recv3(const irc_msg_t *imsg){
    int i = 1;
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    get_cur_time(bmsg->time);
    strncpy(bmsg->nick, imsg->servername, SERVER_LEN);
    strncpy(bmsg->chan, irc.alias, CHAN_LEN);

    while (i < imsg->nparam){
        strcat(bmsg->msg, imsg->param[i++]);
        strcat(bmsg->msg, " ");
    }
    strcat(bmsg->msg, imsg->message);
    if (strlen(bmsg->msg) >= MSG_LEN){
        ERR_FR("message too long");
        goto bad;
    }

    gdk_threads_add_idle((GSourceFunc)ui_msg_recv, bmsg);
    LOG_FR("idle added");
    return;

bad:
    free(bmsg);
    return;
}

static void add_idle_ui_msg_sys(const irc_msg_t *imsg){
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    if (strcmp(imsg->command, "JOIN") == 0
            || strcmp(imsg->command, "PART") == 0){
        if (imsg->nparam != 1){
            ERR_FR("wrong param");
            goto bad;
        }
        strncpy(bmsg->chan, imsg->param[0], CHAN_LEN);
        sprintf(bmsg->msg, "%s %s %s %s",
                imsg->nick, imsg->command, imsg->param[0], imsg->message);

        gdk_threads_add_idle((GSourceFunc)ui_msg_sys, bmsg);
        LOG_FR("idle added");
    }
    return;
bad:
    free(bmsg);
    return;
}

static void add_idle_ui_chan_set_topic(const irc_msg_t *imsg){
    topic_t *topic;

    topic = calloc(1, sizeof(topic_t));
    strncpy(topic->topic, imsg->message, MSG_LEN);
    if (strcmp(imsg->command, "TOPIC") == 0){
        if (imsg->nparam != 1){
            ERR_FR("wrong param");
            return;
        }
        strncpy(topic->chan, imsg->param[0], CHAN_LEN);
    } else {
        if (imsg->nparam != 2){
            ERR_FR("wrong param");
            return;
        }
        strncpy(topic->chan, imsg->param[1], CHAN_LEN);
    }

    gdk_threads_add_idle((GSourceFunc)ui_chan_set_topic, topic);
    LOG_FR("idle added");
    return;
}

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
            else if (strcmp(imsg.command, "NOTICE") == 0
                    || strcmp(imsg.command, RPL_WELCOME) == 0
                    || strcmp(imsg.command, RPL_YOURHOST) == 0
                    || strcmp(imsg.command, RPL_CREATED) == 0
                    || strcmp(imsg.command, RPL_MOTD) == 0
                    || strcmp(imsg.command, RPL_ENDOFMOTD) == 0){
                add_idle_ui_msg_recv2(&imsg);
            }
            else if (strcmp(imsg.command, RPL_MYINFO) == 0
                    || strcmp(imsg.command, RPL_BOUNCE) == 0
                    || strcmp(imsg.command, RPL_LUSEROP) == 0
                    || strcmp(imsg.command, RPL_LUSERUNKNOWN) == 0
                    || strcmp(imsg.command, RPL_LUSERCHANNELS) == 0){
                add_idle_ui_msg_recv3(&imsg);
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
