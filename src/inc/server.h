#ifndef __SERVER_H
#define __SERVER_H

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "srain_msg.h"
#include "irc.h"
// TODO

typedef enum {
    SERVER_UNCONNECTED,
    SERVER_CONNECTING,
    SERVER_CONNECTED,
    SERVER_LOGINED
} ServerStat ;

typedef void (*UIJoinFunc) (GHashTable *buf_table, const char *chan_name);
typedef void (*UIPartFunc) (GHashTable *buf_table, const char *chan_name);
typedef void (*UISysMsgFunc) (GHashTable *buf_table, const char *target, const char *msg, SysMsgType type);
typedef void (*UISendMsgFunc) (GHashTable *buf_table, const char *target, const char *msg);
typedef void (*UIRecvMsgFunc) (GHashTable *buf_table, const char *target, const char *nick, const char *id, const char *msg);
typedef void (*UIUserJoinFunc) (GHashTable *buf_table, const char *chan_name, const char *nick, IRCUserType type, int notify);
typedef void (*UIUserPartFunc) (GHashTable *buf_table, const char *chan_name, const char *nick, const char *reason);
typedef void (*UISetTopicFunc) (GHashTable *buf_table, const char *target, const char *topic);

typedef struct {
    char host[512];
    char port[8];
    IRC irc;
    ServerStat stat;
    GHashTable *buffer_table;
    GThread *listen_thread;

    UIJoinFunc ui_join;
    UIPartFunc ui_part;
    UISysMsgFunc ui_sys_msg;
    UISendMsgFunc ui_send_msg;
    UIRecvMsgFunc ui_recv_msg;
    UIUserJoinFunc ui_user_join;
    UIUserPartFunc ui_user_part;
    UISetTopicFunc ui_set_topic;
} IRCServer;

IRCServer* server_connect(const char *host);
int server_login(IRCServer *server, const char *nick);
int server_join(IRCServer *server, const char *chan_name);
int server_part(IRCServer *server, const char *chan_name, const char *reason);
int server_query(IRCServer *server, const char *target);
int server_unquery(IRCServer *server, const char *target);
int server_send(IRCServer *server, const char *target, char *msg);
void server_close(IRCServer *server);

#endif /* __SERVER_H */
