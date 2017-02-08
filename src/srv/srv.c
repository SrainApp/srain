/**
 * @file srv_name.c
 * @brief SRV module interfaces
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __LOG_ON
// #define __DBG_ON

#include <string.h>
#include <gtk/gtk.h>

#include "srv.h"
#include "srv_event.h"

#include "sirc_cmd.h"

#include "meta.h"
#include "srain.h"
#include "log.h"

Server* server_new(const char *name,
        const char *host,
        int port,
        const char *passwd,
        bool ssl,
        const char *enconding,
        const char *nick,
        const char *username,
        const char *realname){
    if (!host) return NULL;
    if (!nick) return NULL;
    if (!name) name = host;
    if (!passwd) enconding = "";
    if (!enconding) enconding = "UTF-8";
    if (!username) username = PACKAGE_NAME;
    if (!realname) realname = nick;

    Server *srv = g_malloc0(sizeof(Server));

    srv->port = port;
    srv->ssl = ssl;
    srv->stat = SERVER_UNCONNECTED;
    /* srv->chan_list = NULL; */ // by g_malloc0()

    g_strlcpy(srv->name, name, sizeof(srv->name));
    g_strlcpy(srv->host, host, sizeof(srv->host));
    g_strlcpy(srv->passwd, passwd, sizeof(srv->passwd));

    /* srv->user */
    srv->user.me = TRUE;
    g_strlcpy(srv->user.nick, nick, sizeof(srv->user.nick));
    g_strlcpy(srv->user.username, username, sizeof(srv->user.username));
    g_strlcpy(srv->user.realname, realname, sizeof(srv->user.realname));

    /* Get UI & IRC handler */
    srv->ui = sui_new(META_SERVER, srv->host, CHAT_SERVER, srv);
    srv->irc = sirc_new(srv);

    if (!srv->ui || !srv->irc){
        goto bad;
    }

    /* IRC event callbacks */
    srv->irc->events.connect = srv_event_connect;
    srv->irc->events.disconnect = srv_event_disconnect;
    srv->irc->events.ping = srv_event_ping;
    srv->irc->events.welcome = srv_event_welcome;
    srv->irc->events.nick = srv_event_nick;
    srv->irc->events.quit = srv_event_quit;
    srv->irc->events.join = srv_event_join;
    srv->irc->events.part = srv_event_part;
    srv->irc->events.mode = srv_event_mode;
    srv->irc->events.umode = srv_event_umode;
    srv->irc->events.topic = srv_event_topic;
    srv->irc->events.kick = srv_event_kick;
    srv->irc->events.channel = srv_event_channel;
    srv->irc->events.privmsg = srv_event_privmsg;
    srv->irc->events.notice = srv_event_notice;
    srv->irc->events.channel_notice = srv_event_channel_notice;
    srv->irc->events.invite = srv_event_invite;
    srv->irc->events.ctcp_action = srv_event_ctcp_action;
    srv->irc->events.numeric = srv_event_numeric;

    return srv;

bad:
    // server_free();
    return NULL;
}

void server_free(Server *srv){
    if (srv->irc != NULL){
        sirc_free(srv->ui);
    }
    if (srv->ui != NULL){
        sui_free(srv->ui);
    }

    if (srv->chan_list != NULL){
        // free chan list
    }

    g_free(srv);
}

int server_connect(Server *srv){
    sirc_connect(srv->irc, srv->host, srv->port);

    return SRN_OK;
}

void server_disconnect(Server *srv){
    sirc_disconnect(srv);
}

int server_add_chan(Server *srv, const char *name, const char *passwd){
    GList *lst;
    Channel *chan;

    if (!passwd) passwd = "";

    lst = srv->chan_list;
    while (lst) {
        chan = lst->data;
        if (strcasecmp(chan->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    chan = g_malloc0(sizeof(Channel));

    chan->srv = srv;
    chan->me = NULL;
    chan->user_list = NULL;
    chan->ui = sui_new(name, srv->host, CHAT_CHANNEL, srv); // ??

    g_strlcpy(chan->name, name, sizeof(chan->name));
    g_strlcpy(chan->passwd, passwd, sizeof(chan->passwd));

    srv->chan_list = g_list_append(srv->chan_list, chan);

    return SRN_OK;

bad:
    return SRN_ERR;
}

int server_rm_chan(Server *srv, const char *name){
    GList *lst;
    Channel *chan;

    lst = srv->chan_list;
    while (lst) {
        chan = lst->data;
        if (strcasecmp(chan->name, name) == 0){
            sui_free(chan->ui);
            // rm user_list
            g_free(chan);
            srv->chan_list = g_list_delete_link(srv->chan_list, lst);
            return SRN_OK;
        }
    }

    return SRN_ERR;
}

Channel* server_get_chan(Server *srv, const char *name) {
    GList *lst;
    Channel *chan;

    lst = srv->chan_list;
    while (lst) {
        chan = lst->data;
        if (strcasecmp(chan->name, name) == 0){
            return chan;
        }
    }

    return NULL;
}

int server_add_user(Server *srv, const char *chan_name, const char *nick){
    User *user;
    Channel *chan;

    chan = server_get_chan(srv, chan_name);
    if (!chan) {
        return SRN_ERR;
    }

    GList *lst = chan->user_list;
    while (lst){
        user = lst->data;
        if (strcasecmp(user->nick, nick) == 0){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    user = g_malloc0(sizeof(User));

    user->me = FALSE;
    user->type = USER_CHIGUA;

    g_strlcpy(user->nick, nick, sizeof(user->nick));
    // g_strlcpy(user->username, username, sizeof(user->username));
    // g_strlcpy(user->realnaem, realname, sizeof(user->realname));
    chan->user_list = g_list_append(chan->user_list, user);

    sui_add_user(chan->ui, nick, USER_CHIGUA);

    return SRN_OK;
bad:
    return SRN_ERR;
}

void server_rm_user(Server *srv, const char *chan_name, const char *nick){
    User *user;
    Channel *chan;

    chan = server_get_chan(srv, chan_name);
    if (!chan) {
        return SRN_ERR;
    }

    GList *lst = chan->user_list;
    while (lst){
        user = lst->data;
        if (strcasecmp(user->nick, nick) == 0){
            g_free(user);
            g_list_delete_link(chan->user_list, lst);
            sui_rm_user(chan->ui, user->nick);
            return SRN_OK;
        }
        lst = g_list_next(lst);
    }
    //

    return SRN_ERR;
}

User* server_get_user(Server *srv, const char *chan_name, const char *nick){
    User *user;
    Channel *chan;

    chan = server_get_chan(srv, chan_name);
    if (!chan) {
        return SRN_ERR;
    }

    GList *lst = chan->user_list;
    while (lst){
        user = lst->data;
        if (strcasecmp(user->nick, nick) == 0){
            return user;
        }
        lst = g_list_next(lst);
    }

    return NULL;
}

void srv_init(){
    char **argv = { NULL };
    /*
    gtk_init(0, argv);
    Server *srv = server_new("ngircd1", "127.0.0.1", 6667, "", FALSE, "UTF-8",
            "LA", NULL, NULL);
    // Server *srv2 = server_new("ngircd2", "127.0.0.1", 6668, "", FALSE, "UTF-8",
            // "LA2", NULL, NULL);
        server_connect(srv);
        // server_connect(srv2);
        // server_disconnect(srv);
        gtk_main();
        */
}

void srv_finalize(){
}
