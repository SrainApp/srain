#include <glib.h>
#include <time.h>
#include <string.h>
#include <gdk/gdk.h>
#include "ui.h"
#include "log.h"
#include "irc.h"

void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, TIME_LEN, "%m-%d %H:%M", localtime(&curtime));
    timestr[TIME_LEN-1] = '\0';
}

/* add_idle_xxx function receives various parameters and packs them in structure
 * then call xxx funciont as a idle
 */

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

static void add_idle_ui_msg_recv(const char *nick, const char *chan, const char *msg,
        const char *id, const char *img, const char *avatar){
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));
    get_cur_time(bmsg->time);
    strncpy(bmsg->nick, nick, NICK_LEN);
    strncpy(bmsg->chan, chan, CHAN_LEN);
    strncpy(bmsg->msg, msg, MSG_LEN);

    gdk_threads_add_idle((GSourceFunc)ui_msg_recv, bmsg);
    LOG_FR("idle added");
    return;
}

void add_idle_ui_msg_sys(const char *chan, const char *msg){
    bubble_msg_t *bmsg;

    bmsg = calloc(1, sizeof(bubble_msg_t));

    strncpy(bmsg->chan, chan, CHAN_LEN);
    strncpy(bmsg->msg, msg, MSG_LEN);

    gdk_threads_add_idle((GSourceFunc)ui_msg_sys, bmsg);

    LOG_FR("idle added");
    return;
}

void idles_msg_normal(irc_msg_t *imsg){
    if (imsg->nparam != 1){
        ERR_FR("wrong param");
        return;
    }
    if (strncmp(imsg->message, "\1ACTION", 7) == 0){
        /* `/me` process, TODO */
        imsg->message[strlen(imsg->message) - 1] = '\0';
        add_idle_ui_msg_recv(imsg->nick, imsg->param[0], imsg->message + 7, NULL, NULL, NULL);
    } else {
        add_idle_ui_msg_recv(imsg->nick, imsg->param[0], imsg->message, NULL, NULL, NULL);
    }

    LOG_FR("idle added");
    return;
}

/* regard a server message as a PRVIMSG message
 * let nick = servername
 *     msg = param[1..n] + message
 *     chan = irc server's alias
 * then call `add_idle_ui_msg_recv`
 */
void idles_msg_server(const char *server_alias, const irc_msg_t *imsg){
    int i = 1;
    char tmp[MSG_LEN];

    memset(tmp, 0, MSG_LEN*sizeof(char));
    while (i < imsg->nparam){
        strcat(tmp, imsg->param[i++]);
        strcat(tmp, " ");
    }
    strcat(tmp, imsg->message);
    if (strlen(tmp) >= MSG_LEN){
        ERR_FR("message too long");
        return;
    }

    add_idle_ui_msg_recv(imsg->servername, server_alias, tmp, NULL, NULL, NULL);
    LOG_FR("idle added");
    return;
}

void idles_join_part(const irc_msg_t *imsg){
    char tmp[MSG_LEN];

    if (strcmp(imsg->command, "QUIT") == 0){
        snprintf(tmp, MSG_LEN, "%s %s: %s", imsg->nick, imsg->command, imsg->message);
    } else {
        if (strlen(imsg->message) == 0){
            snprintf(tmp, MSG_LEN, "%s %s %s", imsg->nick, imsg->command, imsg->param[0]);
        } else {
            snprintf(tmp, MSG_LEN, "%s %s %s: %s", imsg->nick, imsg->command, imsg->param[0], imsg->message);
        }
    }

    add_idle_ui_msg_sys(imsg->param[0], tmp);

    if (strcmp(imsg->command, "JOIN") == 0){
        add_idle_ui_online_list_add(imsg->param[0], imsg->nick);
    }
    else if (strcmp(imsg->command, "PART") == 0){
        add_idle_ui_online_list_rm(imsg->param[0], imsg->nick);
    } else {
        // TODO
        add_idle_ui_online_list_rm(imsg->param[0], imsg->nick);
    }

    LOG_FR("idle added");
    return;
}

/* transfer a TOPIC/RPL_TOPIC message to topic_t,
 * then call `ui_chan_set_topic`
 */
void idles_topic(const irc_msg_t *imsg){
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
void idles_names(irc_msg_t *imsg){
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
