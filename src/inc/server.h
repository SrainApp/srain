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

typedef void* (*UIAddChanFunc) (const char *srv_name, const char *chan_name);
typedef void (*UIRmChanFunc) (void *chan);
typedef void (*UISysMsgFunc) (void *chan, const char *msg, SysMsgType type);
typedef void (*UISendMsgFunc) (void *chan, const char *msg);
typedef void (*UIRecvMsgFunc) (void *chan, const char *nick, const char *id, const char *msg);
typedef int (*UIUserJoinFunc) (void *chan, const char *nick, IRCUserType type, int notify);
typedef int (*UIUserPartFunc) (void *chan, const char *nick, const char *reason);
typedef void (*UISetTopicFunc) (void *chan, const char *topic);

typedef struct {
    char host[512];
    char port[8];
    IRC irc;
    ServerStat stat;
    GHashTable *chan_table;
    GThread *listen_thread;

    UIAddChanFunc ui_add_chan;
    UIRmChanFunc ui_rm_chan;
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
