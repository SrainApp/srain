/**
 * @file server.c
 * @brief
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */


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
#include "utils.h"
#include "prefs.h"
#include "i18n.h"

SuiAppEvents ui_app_events;
SuiEvents ui_events;
SircEvents irc_events;

void server_init(){
    /* UI event */
    ui_events.disconnect = server_ui_event_disconnect;
    ui_events.send = server_ui_event_send;
    ui_events.join = server_ui_event_join;
    ui_events.part = server_ui_event_part;
    ui_events.query = server_ui_event_query;
    ui_events.unquery = server_ui_event_unquery;
    ui_events.kick = server_ui_event_kick;
    ui_events.invite = server_ui_event_invite;
    ui_events.whois = server_ui_event_whois;
    ui_events.ignore = server_ui_event_ignore;
    ui_events.cutover = server_ui_event_cutover;

    ui_app_events.activate = server_ui_event_activate;
    ui_app_events.connect = server_ui_event_connect;

    /* IRC event */
    irc_events.connect = server_irc_event_connect;
    irc_events.disconnect = server_irc_event_disconnect;
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
    irc_events.ping = server_irc_event_ping;
    irc_events.pong = server_irc_event_pong;
    irc_events.error = server_irc_event_error;
    irc_events.numeric = server_irc_event_numeric;

    server_cmd_init();

    /* Read prefs **/
    SrnRet ret;
    SuiAppPrefs ui_app_prefs = {0};

    ret = prefs_read();
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Prefs read error"), RET_MSG(ret));
    }

    ret = log_read_prefs();
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Prefs read error"), RET_MSG(ret));
    }

    ret = prefs_read_server_prefs_list();
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Prefs read error"), RET_MSG(ret));
    }

    ret = prefs_read_sui_app_prefs(&ui_app_prefs);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Prefs read error"), RET_MSG(ret));
    }

    sui_main_loop(&ui_app_events, &ui_app_prefs);
}

void server_finalize(){
}

Server* server_new_from_prefs(ServerPrefs *prefs){
    Server *srv;

    g_return_val_if_fail(RET_IS_OK(server_prefs_is_valid(prefs)), NULL);
    g_return_val_if_fail(!server_list_get_server(prefs->name), NULL);

    srv = g_malloc0(sizeof(Server));

    srv->registered = FALSE;
    srv->user_quit = FALSE;
    srv->stat = SERVER_DISCONNECTED;
    srv->prefs = prefs;

    srv->chat = chat_new(srv, META_SERVER);
    if (!srv->chat) goto bad;

    srv->user = user_new(srv->chat,
            prefs->nickname,
            prefs->username,
            prefs->realname,
            USER_CHIGUA);
    if (!srv->user) goto bad;
    user_set_me(srv->user, TRUE);

    // FIXME: Corss-required between chat_new() and user_new()
    srv->chat->user = user_ref(srv->user);

    /* NOTE: Ping related issuses are not handled in server.c */
    /* srv->last_pong = 0; */ // by g_malloc0()
    /* srv->ping_timer = 0; */ // by g_malloc0()
    /* srv->delay = 0; */ // by g_malloc0()

    srv->cur_chat = srv->chat;
    /* srv->chat_list = NULL; */ // by g_malloc0()
    /* srv->ignore_list = NULL; */ // by g_malloc0()
    /* srv->brigebot_list = NULL; */ // by g_malloc0()

    /* sirc */
    srv->irc = sirc_new_session(&irc_events, prefs->irc);
    if (!srv->irc) goto bad;
    sirc_set_ctx(srv->irc, srv);

    /* Add into server_list */
    server_list_add(srv);
    return srv;

bad:
    server_free(srv);
    return NULL;
}

void server_free(Server *srv){
    g_return_if_fail(server_list_is_server(srv));

    /* srv->prefs is hold by server_prefs_list, don't free it. */

    if (srv->user != NULL){
        user_free(srv->user);
        srv->user = NULL;
    }

    if (srv->chat != NULL){
        chat_free(srv->chat);
        srv->chat = NULL;
    }

    if (srv->irc != NULL){
        sirc_free_session(srv->irc);
        srv->irc= NULL;
    }

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
        srv->chat_list = NULL;
    }

    /* Remove from server_list */
    server_list_rm(srv);
    g_free(srv);
}

int server_connect(Server *srv){
    g_return_val_if_fail(server_list_is_server(srv), SRN_ERR);
    g_return_val_if_fail(srv->stat == SERVER_DISCONNECTED, SRN_ERR);

    srv->stat = SERVER_CONNECTING;

    sirc_connect(srv->irc, srv->prefs->host, srv->prefs->port);

    return SRN_OK;
}

void server_disconnect(Server *srv){
    g_return_if_fail(server_list_is_server(srv));
    g_return_if_fail(srv->stat == SERVER_CONNECTED);

    sirc_disconnect(srv->irc);
}

/**
 * @brief server_is_registered Whether this server registered
 *
 * @param srv
 *
 * @return TRUE if registered
 */
bool server_is_registered(Server *srv){
    g_return_val_if_fail(server_list_is_server(srv), FALSE);

    return srv->stat == SERVER_CONNECTED && srv->registered == TRUE;
}

void server_wait_until_registered(Server *srv){
    g_return_if_fail(server_list_is_server(srv));

    /* Waiting for connection established */
    while (srv->stat == SERVER_CONNECTING) sui_proc_pending_event();
    /* Waiting until server registered */
    while (srv->stat == SERVER_CONNECTED && srv->registered == FALSE)
        sui_proc_pending_event();
}

int server_add_chat(Server *srv, const char *name){
    GSList *lst;
    Chat *chat;

    g_return_val_if_fail(server_list_is_server(srv), SRN_ERR);

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

    g_return_val_if_fail(server_list_is_server(srv), SRN_ERR);

    lst = srv->chat_list;
    while (lst) {
        chat = lst->data;
        if (strcasecmp(chat->name, name) == 0){
            g_return_val_if_fail(!chat->joined, SRN_ERR);

            if (srv->cur_chat == chat){
                srv->cur_chat = srv->chat;
            }
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

    g_return_val_if_fail(server_list_is_server(srv), NULL);

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

/**
 * @brief server_get_chat_fallback
 *        This function never fail, if name is NULL or not chat found, return
 *        `srv->chat` instead.
 *
 * @param srv
 * @param name
 *
 * @return A instance of Chat
 */
Chat* server_get_chat_fallback(Server *srv, const char *name) {
    Chat *chat;

    g_return_val_if_fail(server_list_is_server(srv), NULL);

    if (!name || !(chat = server_get_chat(srv, name))){
        chat = srv->chat;
    }

    return chat;
}
