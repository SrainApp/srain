/**
 * @file server.c
 * @brief
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __LOG_ON
// #define __DBG_ON

#include <string.h>
#include <gtk/gtk.h>

#include "srv.h"
#include "irc_event.h"
#include "ui_event.h"

#include "sirc_cmd.h"

#include "meta.h"
#include "srain.h"
#include "log.h"

static SuiEvents sui_events;
static SircEvents sirc_events;

void server_init(){
    /* UI event */
    sui_events.connect = ui_event_connect;
    sui_events.send = ui_event_send;
    sui_events.join = ui_event_join;
    sui_events.part = ui_event_part;
    sui_events.query = ui_event_query;
    sui_events.unquery = ui_event_unquery;
    sui_events.kick = ui_event_kick;
    sui_events.invite = ui_event_invite;

    /* UI event */
    sirc_events.connect = irc_event_connect;
    sirc_events.disconnect = irc_event_disconnect;
    sirc_events.ping = irc_event_ping;
    sirc_events.welcome = irc_event_welcome;
    sirc_events.nick = irc_event_nick;
    sirc_events.quit = irc_event_quit;
    sirc_events.join = irc_event_join;
    sirc_events.part = irc_event_part;
    sirc_events.mode = irc_event_mode;
    sirc_events.umode = irc_event_umode;
    sirc_events.topic = irc_event_topic;
    sirc_events.kick = irc_event_kick;
    sirc_events.channel = irc_event_channel;
    sirc_events.privmsg = irc_event_privmsg;
    sirc_events.notice = irc_event_notice;
    sirc_events.channel_notice = irc_event_channel_notice;
    sirc_events.invite = irc_event_invite;
    sirc_events.ctcp_action = irc_event_ctcp_action;
    sirc_events.numeric = irc_event_numeric;
}

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

    server_init();

    Server *srv = g_malloc0(sizeof(Server));

    srv->port = port;
    srv->ssl = ssl;
    srv->stat = SERVER_UNCONNECTED;
    /* srv->chat_list = NULL; */ // by g_malloc0()

    g_strlcpy(srv->name, name, sizeof(srv->name));
    g_strlcpy(srv->host, host, sizeof(srv->host));
    g_strlcpy(srv->passwd, passwd, sizeof(srv->passwd));

    /* srv->user */
    srv->user.me = TRUE;
    g_strlcpy(srv->user.nick, nick, sizeof(srv->user.nick));
    g_strlcpy(srv->user.username, username, sizeof(srv->user.username));
    g_strlcpy(srv->user.realname, realname, sizeof(srv->user.realname));

    /* Get UI & IRC handler */
    srv->ui = sui_new_session(META_SERVER, srv->name, &sui_events,
            SUI_SESSION_SERVER);
    srv->irc = sirc_new_session(&sirc_events);

    if (!srv->ui || !srv->irc){
        goto bad;
    }

    sui_set_ctx(srv->ui, srv);
    sirc_set_ctx(srv->irc, srv);

    /* IRC event callbacks */
    return srv;

bad:
    // server_free();
    return NULL;
}

void server_free(Server *srv){
    if (srv->chat_list != NULL){
        while (srv->chat_list) {
            Chat *chat = srv->chat_list->data;
            server_rm_chat(srv, chat->name);
        }
    }

    if (srv->irc != NULL){
        sirc_free_session(srv->irc);
    }

    if (srv->ui != NULL){
        sui_free_session(srv->ui);
    }

    g_free(srv);
}

int server_connect(Server *srv){
    srv->stat = SERVER_CONNECTING;
    sirc_connect(srv->irc, srv->host, srv->port);

    return SRN_OK;
}

void server_disconnect(Server *srv){
    srv->stat = SERVER_DISCONNECTED;
    sirc_disconnect(srv->irc);
}

int server_add_chat(Server *srv, const char *name, const char *passwd){
    GList *lst;
    Chat *chat;

    if (!passwd) passwd = "";

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    chat = g_malloc0(sizeof(Chat));

    chat->srv = srv;
    chat->me = NULL;
    chat->user_list = NULL;
    chat->ui = sui_new_session(name, srv->name, &sui_events,
            SIRC_IS_CHAN(name) ? SUI_SESSION_CHANNEL : SUI_SESSION_DIALOG); // ??

    g_strlcpy(chat->name, name, sizeof(chat->name));
    g_strlcpy(chat->passwd, passwd, sizeof(chat->passwd));

    srv->chat_list = g_list_append(srv->chat_list, chat);

    return SRN_OK;
}

int server_rm_chat(Server *srv, const char *name){
    GList *lst;
    Chat *chat;

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            sui_free_session(chat->ui);
            // rm user_list
            g_free(chat);
            srv->chat_list = g_list_delete_link(srv->chat_list, lst);
            return SRN_OK;
        }
    }

    return SRN_ERR;
}

Chat* server_get_chat(Server *srv, const char *name) {
    GList *lst;
    Chat *chat;

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return chat;
        }
        lst = g_list_next(lst);
    }

    return NULL;
}

int chat_add_user(Chat *chat, const char *nick, UserType type){
    GList *lst;
    User *user;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (strcasecmp(user->nick, nick) == 0){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    user = g_malloc0(sizeof(User));

    user->me = FALSE;
    user->type = type;

    g_strlcpy(user->nick, nick, sizeof(user->nick));
    // g_strlcpy(user->username, username, sizeof(user->username));
    // g_strlcpy(user->realnaem, realname, sizeof(user->realname));
    chat->user_list = g_list_append(chat->user_list, user);

    sui_add_user(chat->ui, nick, type);

    return SRN_OK;
}

int chat_rm_user(Chat *chat, const char *nick){
    GList *lst;
    User *user;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (strcasecmp(user->nick, nick) == 0){
            chat->user_list = g_list_delete_link(chat->user_list, lst);
            sui_rm_user(chat->ui, user->nick);
            g_free(user);

            return SRN_OK;
        }
        lst = g_list_next(lst);
    }

    return SRN_ERR;
}

User* chat_get_user(Chat *chat, const char *nick){
    User *user;
    GList *lst;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (strcasecmp(user->nick, nick) == 0){
            return user;
        }
        lst = g_list_next(lst);
    }

    return NULL;
}

void srv_init(){ }

void srv_finalize(){ }
