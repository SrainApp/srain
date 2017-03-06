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
#include <strings.h>
#include <gtk/gtk.h>

#include "server.h"
#include "server_irc_event.h"
#include "server_ui_event.h"

#include "sirc/sirc.h"

#include "meta.h"
#include "srain.h"
#include "log.h"

static SuiAppEvents ui_app_events;
static SuiEvents ui_events;
static SircEvents irc_events;
static GList *server_list;

void server_init(){
    /* UI event */
    ui_events.send = server_ui_event_send;
    ui_events.join = server_ui_event_join;
    ui_events.part = server_ui_event_part;
    ui_events.query = server_ui_event_query;
    ui_events.unquery = server_ui_event_unquery;
    ui_events.kick = server_ui_event_kick;
    ui_events.invite = server_ui_event_invite;
    ui_events.whois = server_ui_event_whois;
    ui_events.ignore = server_ui_event_ignore;
    ui_events.disconnect = server_ui_event_disconnect;

    ui_app_events.activate = server_ui_event_activate;
    ui_app_events.connect = server_ui_event_connect;

    /* IRC event */
    irc_events.connect = server_irc_event_connect;
    irc_events.disconnect = server_irc_event_disconnect;
    irc_events.ping = server_irc_event_ping;
    irc_events.welcome = server_irc_event_welcome;
    irc_events.nick = server_irc_event_nick;
    irc_events.quit = server_irc_event_quit;
    irc_events.join = server_irc_event_join;
    irc_events.part = server_irc_event_part;
    irc_events.mode = server_irc_event_mode;
    irc_events.umode = server_irc_event_umode;
    irc_events.topic = server_irc_event_topic;
    irc_events.kick = server_irc_event_kick;
    irc_events.channel = server_irc_event_channel;
    irc_events.privmsg = server_irc_event_privmsg;
    irc_events.notice = server_irc_event_notice;
    irc_events.channel_notice = server_irc_event_channel_notice;
    irc_events.invite = server_irc_event_invite;
    irc_events.ctcp_action = server_irc_event_ctcp_action;
    irc_events.numeric = server_irc_event_numeric;

    server_cmd_init();

    sui_main_loop(&ui_app_events);
}

void server_finalize(){

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
    if (!passwd) passwd = "";
    if (!enconding) enconding = "UTF-8";
    if (!username) username = PACKAGE_NAME;
    if (!realname) realname = nick;

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
    srv->ui = sui_new_session(&ui_events, SUI_SESSION_SERVER);
    srv->irc = sirc_new_session(&irc_events, 0);

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
    Chat *chat;

    if (srv->chat_list != NULL){
        while (srv->chat_list) {
            chat = srv->chat_list->data;
            chat->joined = FALSE; // required by server_rm_chat()
            if (server_rm_chat(srv, chat->name) != SRN_OK){
                break;
            }
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
    sui_start_session(srv->ui, META_SERVER, srv->name);

    return SRN_OK;
}

void server_disconnect(Server *srv){
    if (srv->stat == SERVER_CONNECTED) {
        sirc_disconnect(srv->irc);
    }
}

// TODO: invoked when recv a join QUERY?
int server_add_chat(Server *srv, const char *name, const char *passwd){
    GList *lst;
    Chat *chat;
    bool ischan;

    if (!passwd) passwd = "";

    ischan = sirc_is_chan(name);
    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    chat = g_malloc0(sizeof(Chat));

    chat->joined = FALSE;
    chat->srv = srv;
    chat->me = NULL;
    chat->user_list = NULL;
    chat->ui = sui_new_session(&ui_events,
            ischan ? SUI_SESSION_CHANNEL : SUI_SESSION_DIALOG);

    if (!chat->ui){
        goto bad;
    }

    g_strlcpy(chat->name, name, sizeof(chat->name));
    g_strlcpy(chat->passwd, passwd, sizeof(chat->passwd));

    sui_set_ctx(chat->ui, chat);
    sui_start_session(chat->ui, name, srv->name);

    srv->chat_list = g_list_append(srv->chat_list, chat);

    return SRN_OK;

bad:
    if (chat->ui) {
        sui_free_session(chat->ui);
    }
    if (chat){
        g_free(chat);
    }

    return SRN_ERR;
}

int server_rm_chat(Server *srv, const char *name){
    GList *lst;
    Chat *chat;

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            g_return_val_if_fail(!chat->joined, SRN_ERR);

            sui_free_session(chat->ui);
            // TODO: rm user_list
            g_free(chat);
            srv->chat_list = g_list_delete_link(srv->chat_list, lst);

            return SRN_OK;
        }
        lst = g_list_next(lst);
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
    // user->chat = chat;

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

int user_rename(User *user, const char *new_nick){
    /* Update UI status */
    // if (user->chat) {
        // sui_ren_user(user->chat->ui, user->nick, new_nick, user->type);
    // }

    g_strlcpy(user->nick, new_nick, sizeof(user->nick));

    return SRN_OK;
}

int user_set_type(User *user, UserType type){
    /* Update UI status */
    // if (user->chat) {
        // TODO:
        // return sui_ren_user(user->chat->ui, user->nick, user->nick, type);
    // }

    user->type = type;

    return SRN_OK;
}

Message* message_new(User *origin, const char *content, Context *ctx){
    Message *msg;

    g_return_val_if_fail(content, NULL);

    msg = g_malloc0(sizeof(Message));

    msg->origin = origin;
    if (origin) {
        msg->dname = g_strdup(origin->nick);
    }
    // msg->role = NULL; // via g_malloc0()
    msg->content = g_strdup(content);
    msg->dcontent = g_strdup(content);
    LOG_FR(": %s", msg->dcontent);
    // msg->time = ...;
    msg->mentioned = FALSE;
    msg->ctx = ctx;
    // msg->urls = NULL; // via  g_malloc0()
    // msg->ui = NULL; // via  g_malloc0()

    return msg;
}

void message_free(Message *msg){
    g_return_if_fail(msg);

    if (msg->origin) {
        // Free User? NO.
    }

    if (msg->urls) {
        // TODO
    }

    if (msg->dname) {
        g_free(msg->dname);
    }

    if (msg->role) {
        g_free(msg->role);
    }

    if (msg->content) {
        g_free(msg->content);
    }

    if (msg->dcontent) {
        g_free(msg->dcontent);
    }
    // if (msg->ui) {...}

    g_free(msg);
}
