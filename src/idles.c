#include <glib.h>
#include <time.h>
#include <string.h>
#include <gdk/gdk.h>
#include "ui.h"
#include "log.h"
#include "irc.h"

static void add_idle_ui_online_list_add(const char *chan, const char *nick){
    online_list_item_t *item; 

    item = calloc(1, sizeof(online_list_item_t));
    strncpy(item->chan, chan, CHAN_LEN);
    strncpy(item->nick, nick, NICK_LEN);
    gdk_threads_add_idle((GSourceFunc)ui_online_list_add, item);

    return;
}

static void add_idle_ui_online_list_rm(const char *chan, const char *nick){
    online_list_item_t *item; 

    item = calloc(1, sizeof(online_list_item_t));
    strncpy(item->chan, chan, CHAN_LEN);
    strncpy(item->nick, nick, NICK_LEN);
    gdk_threads_add_idle((GSourceFunc)ui_online_list_rm, item);

    return;
}

void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, TIME_LEN, "%m-%d %H:%M", localtime(&curtime));
    timestr[TIME_LEN-1] = '\0';
}

/* transfer a standard PRIVMSG message to bubble_msg_t,
 * then call `ui_msg_recv`
 */
void add_idle_ui_msg_recv(const irc_msg_t *imsg){
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
void add_idle_ui_msg_recv2(const char *chan, const irc_msg_t *imsg){
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    get_cur_time(bmsg->time);
    strncpy(bmsg->nick, imsg->servername, SERVER_LEN);
    strncpy(bmsg->chan, chan, CHAN_LEN);
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
void add_idle_ui_msg_recv3(const char *chan, const irc_msg_t *imsg){
    int i = 1;
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    get_cur_time(bmsg->time);
    strncpy(bmsg->nick, imsg->servername, SERVER_LEN);
    strncpy(bmsg->chan, chan, CHAN_LEN);

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

void add_idle_ui_msg_sys(const irc_msg_t *imsg){
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    if (strcmp(imsg->command, "JOIN") == 0
            || strcmp(imsg->command, "PART") == 0){
        if (imsg->nparam != 1){
            ERR_FR("wrong param");
            goto bad;
        }
        strncpy(bmsg->chan, imsg->param[0], CHAN_LEN);
        sprintf(bmsg->msg, "%s %s %s %s", imsg->nick, imsg->command, imsg->param[0], imsg->message);

        gdk_threads_add_idle((GSourceFunc)ui_msg_sys, bmsg);

        if (strcmp(imsg->command, "JOIN") == 0){
            add_idle_ui_online_list_add(imsg->param[0], imsg->nick);
        } else {
            add_idle_ui_online_list_rm(imsg->param[0], imsg->nick);
        }
        LOG_FR("idle added");
    }
    return;
bad:
    free(bmsg);
    return;
}

/* transfer a TOPIC/RPL_TOPIC message to topic_t,
 * then call `ui_chan_set_topic`
 */
void add_idle_ui_chan_set_topic(const irc_msg_t *imsg){
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

/* separate a RPL_NAMREPLY message to mulitple nickname
 * then call `add_ui_online_list_add` for every nickname
 */
void add_idle_ui_online_list_init(irc_msg_t *imsg){
    int i = 0;
    char *nickptr;

    if (imsg->nparam != 3){
        ERR_FR("wrong param");
        return;
    }
    nickptr = strtok(imsg->message, " ");
    while (nickptr){
        add_idle_ui_online_list_add(imsg->param[2], nickptr);
        i++;
        nickptr = strtok(NULL, " ");
    }

    LOG_FR("%d idle(s) added", i);
    return;
}

