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
#include "srv_session_cmd.h"
#include "srv_test.h"

#include "irc.h"
#include "irc_cmd.h"

#include "ui.h"

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
    srv->fd = -1;
    srv->ui = ui_add_chat(srv, META_SERVER, CHAT_SERVER);

    if (!srv->ui){
        goto bad;
    }

    g_strlcpy(srv->name, name, sizeof(srv->name));
    g_strlcpy(srv->host, host, sizeof(srv->host));
    g_strlcpy(srv->passwd, passwd, sizeof(srv->passwd));

    /* srv->user */
    srv->user.me = TRUE;
    g_strlcpy(srv->user.nick, nick, sizeof(srv->user.nick));
    g_strlcpy(srv->user.username, username, sizeof(srv->user.username));
    g_strlcpy(srv->user.realname, realname, sizeof(srv->user.realname));

    g_mutex_init(&srv->mutex);

    return srv;

bad:
    // server_free();
    return NULL;
}

void server_free(Server *srv){
    if (srv->fd != -1){
        // close connection...
    }

    if (srv->ui != NULL){
        ui_rm_chat(srv->ui);
    }

    if (srv->chan_list != NULL){
        // free chan list
    }

    g_free(srv);
}

int server_connect(Server *srv){
    User *user = &srv->user;
    irc_connect(srv);

    while (srv->stat == SERVER_CONNECTING) {
        while (gtk_events_pending()) gtk_main_iteration();
    }

    if (srv->stat == SERVER_DISCONNECTED) {
        ERR_FR("Failed to connect to server %s(%s:%d)",
                srv->name, srv->host, srv->port);
        return SRN_ERR;
    }

    irc_cmd_nick(srv->fd, user->nick);
    irc_cmd_user(srv->fd, user->username, "hostname", "servername", user->realname);
}

void server_disconnect(Server *srv){
    irc_disconnect(srv);
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
    chan->ui = ui_add_chat(srv, name, CHAT_CHANNEL); // TODO

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
            // ui_rm_chat
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

    // ui_add_user()

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
            // ui_rm_user()
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
    irc_init();
    char **argv = { NULL };
    // gtk_init(0, argv);
    Server *srv = server_new("ngircd1", "127.0.0.1", 6667, "", FALSE, "UTF-8",
            "LA", NULL, NULL);
    Server *srv2 = server_new("ngircd2", "127.0.0.1", 6668, "", FALSE, "UTF-8",
            "LA2", NULL, NULL);
    if (srv) {
        server_connect(srv);
        server_connect(srv2);
        // server_disconnect(srv);
        // gtk_main();
    }
}

void srv_finalize(){
}
