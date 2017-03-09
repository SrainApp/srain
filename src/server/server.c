/**
 * @file server.c
 * @brief
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __LOG_ON
#define __DBG_ON

#include <string.h>
#include <strings.h>
#include <gtk/gtk.h>

#include "server.h"
#include "server_cmd.h"
#include "server_irc_event.h"
#include "server_ui_event.h"

#include "sirc/sirc.h"

#include "meta.h"
#include "srain.h"
#include "log.h"

SuiAppEvents ui_app_events;
SuiEvents ui_events;
SircEvents irc_events;

static GSList *server_list;

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
    // TODO: ad-hoc method
    srv->user.srv = srv;
    srv->user.chat = NULL;
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
    server_free(srv);
    return NULL;
}

void server_free(Server *srv){
    GSList *lst;

    /* Free chat list */
    if (srv->chat_list){
        lst = srv->chat_list;
        while (lst){
            if (lst->data){
                chat_free(lst->data);
                lst->data = NULL;
            }
            lst = g_slist_next(lst);
        }
        g_slist_free(srv->chat_list);
    }

    if (srv->irc != NULL){
        sirc_free_session(srv->irc);
        srv->irc= NULL;
    }

    if (srv->ui != NULL){
        sui_free_session(srv->ui);
        srv->ui = NULL;
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
int server_add_chat(Server *srv, const char *name){
    GSList *lst;
    Chat *chat;

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    chat = chat_new(srv, name);
    g_return_val_if_fail(chat, SRN_ERR);

    srv->chat_list = g_slist_append(srv->chat_list, chat);

    return SRN_OK;
}

int server_rm_chat(Server *srv, const char *name){
    GSList *lst;
    Chat *chat;

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            g_return_val_if_fail(!chat->joined, SRN_ERR);

            chat_free(chat);
            srv->chat_list = g_slist_delete_link(srv->chat_list, lst);

            return SRN_OK;
        }
        lst = g_slist_next(lst);
    }

    return SRN_ERR;
}

Chat* server_get_chat(Server *srv, const char *name) {
    GSList *lst;
    Chat *chat;

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            return chat;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}
