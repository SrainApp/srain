/**
 * @file server_intf.c
 * @brief UI interface for server module
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-20
 */
#define __LOG_ON

#include <string.h>
#include <glib.h>
#include "server.h"
#include "server_cmd.h"
#include "srain_chan.h"
#include "srain_app.h"
#include "log.h"


void server_intf_ui_add_chan(IRCServer *srv, const char *chan_name){
    void *chan;

    LOG_FR("server: %s, chan_name: %s", srv->host, chan_name);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan){
        ERR_FR("%s already exist", chan_name);
        return;
    }

    chan = srv->ui_add_chan(srv, srv->host, chan_name);

    g_hash_table_insert(srv->chan_table, strdup(chan_name), chan);
}

void server_intf_ui_rm_chan(IRCServer *srv, const char *chan_name){
    void *chan;

    LOG_FR("server: %s, chan_name: %s", srv->host, chan_name);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s not found", chan_name);
        return;
    }

    srv->ui_rm_chan(chan);
    g_hash_table_remove(srv->chan_table, chan_name);
}

void server_intf_ui_sys_msg (IRCServer *srv, const char *chan_name,
        const char *msg, SysMsgType type){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, msg: '%s', type: %d",
            srv->host, chan_name, msg, type);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s not found", chan_name);
        return;
    }

    // TODO
    srv->ui_sys_msg(chan, msg, type);
}

void server_intf_ui_send_msg(IRCServer *srv, const char *chan_name, const char *msg){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, msg: '%s'", srv->host, chan_name, msg);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s not found", chan_name);
        return;
    }

    srv->ui_send_msg(chan, msg);
}

void server_intf_ui_recv_msg(IRCServer *srv, const char *chan_name,
        const char *nick, const char *id, const char *msg){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, nick: %s, id: %s, msg: '%s'",
            srv->host, chan_name, nick, id, msg);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    srv->ui_recv_msg(chan, nick, id, msg);
}

void server_intf_ui_user_join(IRCServer *srv, const char *chan_name,
        const char *nick, IRCUserType type, int notify){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, nick: %s, type: %d",
            srv->host, chan_name, nick, type);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    srv->ui_user_join(chan, nick, type, notify);
}

void server_intf_ui_user_part(IRCServer *srv, const char *chan_name,
        const char *nick, const char *reason){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, nick: %s, reasion: %s",
            srv->host, chan_name, nick, reason);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    srv->ui_user_part(chan, nick, reason);
}

void server_intf_ui_set_topic(IRCServer *srv, const char *chan_name, const char *topic){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, topic: '%s'",
            srv->host, chan_name, topic);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    srv->ui_set_topic(chan, topic);
}
