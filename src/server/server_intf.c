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

/**
 * @brief server_intf_ui_sys_msg
 *
 * @param srv
 * @param chan_name can be NULL, srv->ui_sys_msg will deal with it
 * @param msg
 * @param type
 */
void server_intf_ui_sys_msg (IRCServer *srv, const char *chan_name,
        const char *msg, SysMsgType type){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, msg: '%s', type: %d",
            srv->host, chan_name, msg, type);

    chan = NULL;
    if (chan_name){
        chan = g_hash_table_lookup(srv->chan_table, chan_name);
        if (chan == NULL){
            ERR_FR("%s not found", chan_name);
        }
    }

    srv->ui_sys_msg(chan, msg, type);
}

/* formatted output version of server_intf_ui_sys_msg */
void server_intf_ui_sys_msgf(IRCServer *srv, const char *chan_name,
        SysMsgType type, const char *fmt, ...){
    char msg[512];
    va_list args;

    if (strlen(fmt) != 0){
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);
    }

    server_intf_ui_sys_msg(srv, chan_name, msg, type);
}

/* broadcast version of server_intf_ui_sys_msgf */
void server_intf_ui_sys_msgf_bcst(IRCServer *srv, SysMsgType type,
        const char *fmt, ...){
    char msg[512];
    va_list args;
    GHashTableIter iter;
    gpointer key, value;

    if (strlen(fmt) != 0){
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);
    }

    g_hash_table_iter_init (&iter, srv->chan_table);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        server_intf_ui_sys_msg(srv, key, msg, type);
    }
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

void server_intf_ui_user_list_add(IRCServer *srv, const char *chan_name,
        const char *nick, IRCUserType type, int notify){
    int res;
    void *chan;

    LOG_FR("server: %s, chan_name: %s, nick: %s, type: %d",
            srv->host, chan_name, nick, type);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    if (srv->ui_user_list_add(chan, nick, type) != -1 && notify){
        server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_NORMAL,
                "%s has joined %s", nick, chan_name);
    }
}

void server_intf_ui_user_list_rm(IRCServer *srv, const char *chan_name,
        const char *nick, const char *reason){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, nick: %s, reasion: %s",
            srv->host, chan_name, nick, reason);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    if (srv->ui_user_list_rm(chan, nick, reason) != -1){
        server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_NORMAL,
                "%s has left %s: %s", nick, chan_name, reason);
    }
}


/* called when recvied a QUIT message */
void server_intf_ui_user_list_rm_bcst(IRCServer *srv, const char *nick, const char *reason){
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, srv->chan_table);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        server_intf_ui_user_list_rm(srv, key, nick, reason);
    }
}

void server_intf_ui_user_list_rename(IRCServer *srv, const char *chan_name,
        const char *old_nick, const char *new_nick){
    void *chan;

    LOG_FR("server: %s, chan_name: %s, old_nick: %s, new_nick: %s",
            srv->host, chan_name, old_nick, new_nick);

    chan = g_hash_table_lookup(srv->chan_table, chan_name);
    if (chan == NULL){
        ERR_FR("%s no found", chan_name);
        return;
    }

    if (srv->ui_user_list_rename(chan, old_nick, new_nick) != -1){
        server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_NORMAL,
                "%s is now known as %s", old_nick, new_nick);
    }
}

void server_intf_ui_user_list_rename_bcst(IRCServer *srv, const char *old_nick, const char *new_nick){
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, srv->chan_table);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        server_intf_ui_user_list_rename(srv, key, old_nick, new_nick);
    }
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
