/**
 * @file server.c
 * @brief irc server manange and message dispatch
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-15
 *
 * This file gules UI module and IRC module together
 * and provides some abstract operations of a IRC client:
 *      connect to server, login as someone, join a channel,
 *      part from a channel, send message, receive message and etc.
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <gtk/gtk.h>
#include "srain_chan.h"
#include "irc.h"
#include "server.h"
#include "server_msg_dispatch.h"
#include "meta.h"
#include "log.h"
#include "filter.h"
#include "str_list.h"

#define __LOG_ON

GList *host_list;

static char* str_to_lowcase(char *str){
    int i;

    for (i = 0; i < strlen(str); i++){
        str[i] = tolower(str[i]);
    }

    return str;
}

static IRCServer *server_new(const char *host, const char *port){
    IRCServer *server;

    server = calloc(sizeof(IRCServer), 1);

    server->stat = SERVER_UNCONNECTED;
    server->buffer_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

    strncpy(server->host, host, sizeof(server->host));
    strncpy(server->port, port, sizeof(server->port));

    // TODO: func pointer assign
    return server;
}

static void server_free(IRCServer *server){
    // if the listen_thread is still running, IRCServer
    // can not be freed.
    if (server->listen_thread != NULL) return;

    server->stat = SERVER_UNCONNECTED;

    // TODO: free key and value
    g_hash_table_destroy(server->buffer_table);

    free(server);

    return;
}

void server_recv(IRCServer *server);

void _server_connect(IRCServer *server){
    server->stat = SERVER_CONNECTING;

    if (irc_connect(&(server->irc), server->host, server->port) == 0){
        server->stat = SERVER_CONNECTING;
        server_recv(server);   // endless loop
    } else {
        server->stat = SERVER_UNCONNECTED;
    }
}

IRCServer* server_connect(const char *host){
    IRCServer *server;

    if (str_list_add(host_list, host) != 0){
        return NULL;
    }
    // TODO
    server = server_new(host, "6666");
    server->ui_join(server->buffer_table, META_SERVER);
    server->listen_thread = g_thread_new(NULL,
            (GThreadFunc)_server_connect, server);

    while (server->stat == SERVER_CONNECTING)
        while (gtk_events_pending()) gtk_main_iteration();

    if (server->stat == SERVER_CONNECTED){
        return server;
    } else {
        ERR_FR("connection failed");
        server->listen_thread = NULL;
        server_free(server);

        return NULL;
    }
}

int server_login(IRCServer *server, const char *nick){
    if (server->stat != SERVER_CONNECTED) {
        return -1;
    }

    if (irc_login(&(server->irc), nick) >= 0){
        // TODO: stat = SERVER_LOGINED when received a logined message
        server->stat = SERVER_LOGINED;
        return 0;
    }

    return -1;
}

/* this function work in listening thread */
void server_recv(IRCServer *server){
    IRCMsg *imsg;
    IRCMsgType type;

    LOG_FR("%s", server->host);

    for (;;){
        imsg = calloc(1, sizeof(IRCMsg));
        type = irc_recv(&(server->irc), imsg);

        if (type == IRCMSG_SCKERR){
            irc_close(&(server->irc));
            // TODO
            return;
        }
        if (type == IRCMSG_MSG){
            // let main loop process data
            imsg->server = server;
            gdk_threads_add_idle((GSourceFunc)server_msg_dispatch, imsg);
        } else {
            free(imsg);
        }
    }
}

int server_join(IRCServer *server, const char *chan_name){
    return irc_join_req(&(server->irc), chan_name);
}

int server_part(IRCServer *server, const char *chan_name, const char *reason){
    if (!reason) reason = META_NAME_VERSION;

    return irc_part_req(&(server->irc), chan_name, reason);
}

int server_query(IRCServer *server, const char *target){
    char *person;

    if (server->stat != SERVER_LOGINED){
        return -1;
    }

    if (IS_CHAN(target)){
        return server_join(server, target);
    }

    person = str_to_lowcase(strdup(target));
    if (g_hash_table_lookup(server->buffer_table, target) != NULL){
        free(person);
        return -1;
    }

    // ui_chan_add(server, person);
    // TODO
    free(person);

    return 1;
}

int server_unquery(IRCServer *server, const char *target){
    char *person;
    SrainChan *chan;

    if (server->stat != SERVER_LOGINED){
        return -1;
    }

    if (IS_CHAN(target)){
        return server_part(server, target, NULL);
    }

    person = str_to_lowcase(strdup(target));
    if ((chan = g_hash_table_lookup(server->buffer_table, target)) == NULL){
        free(person);
        return -1;
    }

    // ui_chan_rm(server, person);
    // TODO

    free(person);

    return 0;
}

int server_send(IRCServer *server, const char *chan_name, char *msg){
    LOG_FR("chan: '%s', msg: '%s'", chan, msg);

    server->ui_send_msg(server->buffer_table, chan_name, msg);

    if (irc_send(&(server->irc), chan_name, msg, 0) <= 0){
        // ui_msg_sysf(NULL, SYS_MSG_ERROR, "faild to send message \"%.8s...\"", msg);
        return -1;
    }

    return 0;
}


void server_close(IRCServer *server){
    if (server->stat != SERVER_LOGINED || server->stat != SERVER_CONNECTED) {
        return;
    }

    irc_quit_req(&(server->irc), META_NAME_VERSION);
    irc_close(&(server->irc));

    server_free(server);
}

