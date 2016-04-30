/**
 * @file server.c
 * @brief irc servers manange
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-15
 */

#define __LOG_ON

#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <gtk/gtk.h>
#include "srain_app.h"
#include "irc.h"
#include "server.h"
#include "server_msg_dispatch.h"
#include "meta.h"
#include "log.h"
#include "filter.h"
#include "str_list.h"
#include "server_intf.h"

GList *host_list = NULL;

static IRCServer *server_new(const char *host, const char *port){
    IRCServer *srv;

    srv = calloc(sizeof(IRCServer), 1);

    srv->stat = SERVER_UNCONNECTED;
    srv->chan_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

    strncpy(srv->host, host, 512);
    strncpy(srv->port, port, 8);

    srv->ui_add_chan = (UIAddChanFunc)srain_app_add_chan;
    srv->ui_rm_chan = (UIRmChanFunc)srain_app_rm_chan;
    srv->ui_sys_msg = (UISysMsgFunc)srain_app_sys_msg;
    srv->ui_send_msg = (UISendMsgFunc)srain_app_send_msg;
    srv->ui_recv_msg = (UIRecvMsgFunc)srain_app_recv_msg;
    srv->ui_user_list_add = (UIUserListAddFunc)srain_app_user_list_add;
    srv->ui_user_list_rm = (UIUserListRmFunc)srain_app_user_list_rm;
    srv->ui_user_list_rename = (UIUserListRenameFunc)srain_app_user_list_rename;
    srv->ui_set_topic = (UISetTopicFunc)srain_app_set_topic;

    return srv;
}

static void server_free(IRCServer *srv){
    // if the listen_thread is still running, IRCServer
    // can not be freed.
    if (srv->listen_thread != NULL) return;

    srv->stat = SERVER_UNCONNECTED;

    // TODO: free key and value
    g_hash_table_destroy(srv->chan_table);

    free(srv);

    return;
}

void server_recv(IRCServer *srv);

void _server_connect(IRCServer *srv){
    if (irc_connect(&(srv->irc), srv->host, srv->port) == 0){
        srv->stat = SERVER_CONNECTED;
        server_recv(srv);   // endless loop
    } else {
        srv->stat = SERVER_UNCONNECTED;
    }
}

IRCServer* server_connect(const char *host){
    IRCServer *srv;

    if (str_list_find(host_list, host)){
        ERR_FR("you have connected to %s", host);
        return NULL;
    }

    srv = server_new(host, "6666");
    server_intf_ui_add_chan(srv, META_SERVER);

    srv->stat = SERVER_CONNECTING;
    srv->listen_thread = g_thread_new(NULL,
            (GThreadFunc)_server_connect, srv);

    while (srv->stat == SERVER_CONNECTING)
        while (gtk_events_pending()) gtk_main_iteration();

    if (srv->stat == SERVER_CONNECTED){
        host_list = str_list_add(host_list, host);
        LOG_FR("%s connected", host);

        return srv;
    } else {
        ERR_FR("connection failed");
        srv->listen_thread = NULL;

        return NULL;
    }
}

int server_login(IRCServer *srv, const char *nick){
    LOG_FR("srv: %s, nick: %s", srv ? srv->host : "(null)", nick);

    if (srv->stat != SERVER_CONNECTED) {
        return -1;
    }

    if (irc_login(&(srv->irc), nick) >= 0){
        // TODO: stat = SERVER_LOGINED when received a logined message
        srv->stat = SERVER_LOGINED;
        return 0;
    }

    return -1;
}

/************************************************************
 * NOTE: This function works in a number of different threads
 * non-thread-local static varible is NOT allowed.
 ************************************************************/
void server_recv(IRCServer *srv){
    IRCMsg *imsg;
    IRCMsgType type;

    LOG_FR("%s", srv->host);

    for (;;){
        imsg = calloc(1, sizeof(IRCMsg));
        type = irc_recv(&(srv->irc), imsg);

        if (type == IRCMSG_SCKERR){
            irc_close(&(srv->irc));
            // TODO: free IRCServer
            return;
        }
        if (type == IRCMSG_MSG){
            // let main loop process data
            imsg->server = srv;
            gdk_threads_add_idle((GSourceFunc)server_msg_dispatch, imsg);
        } else {
            free(imsg);
        }
    }
}

int server_join(IRCServer *srv, const char *chan_name){
    return irc_join_req(&(srv->irc), chan_name);
}

int server_part(IRCServer *srv, const char *chan_name, const char *reason){
    if (!reason) reason = META_NAME_VERSION;

    return irc_part_req(&(srv->irc), chan_name, reason);
}

int server_query(IRCServer *srv, const char *target){
    if (srv->stat != SERVER_LOGINED){
        return -1;
    }

    if (IS_CHAN(target)){
        return server_join(srv, target);
    }

    if (strcasecmp(target, META_SERVER) == 0){
        ERR_FR("can not query META_SERVER");
        return -1;
    }

    server_intf_ui_add_chan(srv, target);
    // TODO: add whois target
    return 0;
}

int server_unquery(IRCServer *srv, const char *target){
    if (srv->stat != SERVER_LOGINED){
        return -1;
    }

    if (IS_CHAN(target)){
        return server_part(srv, target, NULL);
    }

    if (strcasecmp(target, META_SERVER) == 0){
        ERR_FR("can not unquery META_SERVER");
        return -1;
    }

    server_intf_ui_rm_chan(srv, target);
    return 0;
}

int server_send(IRCServer *srv, const char *chan_name, char *msg){
    LOG_FR("chan: '%s', msg: '%s'", chan_name, msg);

    server_intf_ui_send_msg(srv, chan_name, msg);

    if (irc_send(&(srv->irc), chan_name, msg, 0) <= 0){
        server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_ERROR,
                "faild to send message \"%.8s...\"", msg);
        return -1;
    }

    return 0;
}

void server_quit_req(IRCServer *srv){
    LOG_FR("host: %s", srv->host);

    // if (srv->stat != SERVER_LOGINED || srv->stat != SERVER_CONNECTED) {
        //return;
    // }

    /* if socket error */
    if (irc_quit_req(&(srv->irc), META_NAME_VERSION) < 0){
        server_quit_ack(srv);
    }
}

void server_quit_ack(IRCServer *srv){
    LOG_FR("host: %s", srv->host);

    server_intf_ui_rm_all_chan(srv);
    server_free(srv);
}
