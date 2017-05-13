#define __DBG_ON
#define __LOG_ON

#include <string.h>

#include "server.h"
#include "server_cmd.h"
#include "server_ui_event.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "rc.h"
#include "filter.h"

static Server* ctx_get_server(SuiSession *sui);
static Chat* ctx_get_chat(SuiSession *sui);

void server_ui_event_activate(SuiEvent event, const char *params[], int count){
    rc_read();
    // TODO: welcome or command error report
}

void server_ui_event_connect(SuiEvent event, const char *params[], int count){
    Server *srv;
    SircSessionFlag ircflag;

    g_return_if_fail(count == 9);
    const char *name = params[0];
    const char *host = params[1];
    int port = atoi(params[2]);
    const char *passwd = params[3];
    bool ssl = strcmp(params[4], "TRUE") == 0 ? TRUE : FALSE;
    bool notverify = strcmp(params[5], "TRUE") == 0 ? TRUE : FALSE;
    const char *encoding = params[6];
    const char *nick = params[7];
    const char *realname= params[8];
    const char *username = PACKAGE_NAME;

    if (strlen(realname) == 0) {
        realname = "Can you can a can?";
    }

    ircflag = 0;
    if (ssl) ircflag |= SIRC_SESSION_SSL;
    if (notverify) ircflag |= SIRC_SESSION_SSL_NOTVERIFY;

    srv = server_new(name, host, port, passwd, encoding, ircflag, nick, username, realname);
    g_return_if_fail(srv);

    server_connect(srv);
}

void server_ui_event_disconnect(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;

    g_return_if_fail(count == 0);

    srv = ctx_get_server(sui);
    g_return_if_fail(srv);

    if (sirc_cmd_quit(srv->irc, "QUIT") == SRN_ERR){
        server_free(srv);
    }
}

void server_ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *msg;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    msg = params[0];

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(srv);
    g_return_if_fail(chat);

    // Command or message?
    if (msg[0] == '/'){
        if (server_cmd(chat, msg) == SRN_OK){
            // Nothing to do.
        }
    } else {
        chat_add_sent_message(chat, msg);
    }
}

void server_ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    const char *passwd;
    Server *srv;

    g_return_if_fail(count == 1 || count == 2);
    name = params[0];
    passwd = count == 2 ? params[1] : NULL;

    srv = ctx_get_server(sui);
    g_return_if_fail(srv);

    sirc_cmd_join(srv->irc, name, passwd);
}

void server_ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 0);
    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(srv);
    g_return_if_fail(chat);

    if (chat->joined) {
        sirc_cmd_part(srv->irc, chat->name, "Leave.");
    } else {
        server_rm_chat(srv, chat->name);
    }
}

void server_ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    Server *srv;

    g_return_if_fail(count == 1);
    name = params[0];

    srv = ctx_get_server(sui);
    g_return_if_fail(srv);

    server_add_chat(srv, name);
}

void server_ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(srv);
    g_return_if_fail(chat);

    server_rm_chat(srv, chat->name);
}

void server_ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(srv);
    g_return_if_fail(chat);

    sirc_cmd_kick(srv->irc, nick, chat->name, "Kick.");
}

void server_ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_if_fail(srv);
    g_return_if_fail(chat);

    sirc_cmd_invite(srv->irc, nick, chat->name);
}

void server_ui_event_whois(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;

    g_return_if_fail(count == 1);
    nick = params[0];

    srv = ctx_get_server(sui);
    g_return_if_fail(srv);

    sirc_cmd_whois(srv->irc, nick);
}

void server_ui_event_ignore(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    chat = ctx_get_chat(sui);
    g_return_if_fail(chat);

    nick_filter_add_nick(chat, nick);
}

void server_ui_event_cutover(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 0);

    srv = ctx_get_server(sui);
    g_return_if_fail(srv);
    chat = ctx_get_chat(sui);
    g_return_if_fail(chat);

    srv->cur_chat = chat;
}

/* Get a Server object from SuiSession context (sui->ctx) */
static Server* ctx_get_server(SuiSession *sui){
    void *ctx;
    Chat *chat;

    ctx = sui_get_ctx(sui);
    g_return_val_if_fail(ctx, NULL);

    chat = ctx;

    return chat->srv;
}

/* Get a Chat object from SuiSession context (sui->ctx) */
static Chat* ctx_get_chat(SuiSession *sui){
    return sui_get_ctx(sui);
}
